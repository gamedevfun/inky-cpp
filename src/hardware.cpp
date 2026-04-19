#include "inky/hardware.hpp"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <thread>
#include <stdexcept>
#include <string>
#include <unordered_map>

#ifdef INKY_ENABLE_LINUX_GPIOD
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <gpiod.h>
#include <cerrno>
#endif

namespace inky {

MemoryGpio::MemoryGpio(std::size_t lines) : states_(lines, false), configs_(lines) {}

void MemoryGpio::configureOutput(unsigned int line, bool initialState) {
    ensureSize(line);
    configs_[line] = LineConfig{true, false, GpioEdge::None};
    states_[line] = initialState;
}

void MemoryGpio::configureInput(unsigned int line, bool pullUp, GpioEdge edge) {
    ensureSize(line);
    configs_[line] = LineConfig{false, pullUp, edge};
    states_[line] = pullUp;
}

void MemoryGpio::setValue(unsigned int line, bool active) {
    ensureSize(line);
    const bool previous = states_[line];
    states_[line] = active;
    if (!configs_[line].isOutput && configs_[line].edge == GpioEdge::Falling && previous && !active) {
        edgeEvents_.push_back(line);
    }
}

bool MemoryGpio::getValue(unsigned int line) const {
    return line < states_.size() ? states_[line] : false;
}

std::optional<unsigned int> MemoryGpio::waitForEdge(std::span<const unsigned int> lines, std::chrono::milliseconds timeout) {
    const auto deadline = std::chrono::steady_clock::now() + timeout;

    while (true) {
        while (!edgeEvents_.empty()) {
            const unsigned int eventLine = edgeEvents_.front();
            edgeEvents_.pop_front();
            if (std::find(lines.begin(), lines.end(), eventLine) != lines.end()) {
                return eventLine;
            }
        }

        if (std::chrono::steady_clock::now() >= deadline) {
            return std::nullopt;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void MemoryGpio::ensureSize(unsigned int line) {
    if (line >= states_.size()) {
        states_.resize(line + 1, false);
        configs_.resize(line + 1);
    }
}

#ifdef INKY_ENABLE_LINUX_GPIOD
namespace {
std::runtime_error systemError(const std::string &context) {
    return std::runtime_error(context + ": " + std::strerror(errno));
}
}

struct LinuxGpio::Impl {
    enum class RequestMode {
        Output,
        Input,
        FallingEdgeInput,
    };

    struct LineHandle {
        gpiod_line *line{nullptr};
        RequestMode mode{RequestMode::Input};
        bool pullUp{false};
    };

    explicit Impl(const std::string &chipPath) {
        chip = gpiod_chip_open(chipPath.c_str());
        if (chip == nullptr) {
            throw systemError("gpiod_chip_open");
        }
    }

    ~Impl() {
        for (auto &entry : lines) {
            gpiod_line_release(entry.second.line);
        }
        if (chip != nullptr) {
            gpiod_chip_close(chip);
        }
    }

    void requestLine(unsigned int line, RequestMode mode, bool initialState, bool pullUp) {
        auto existing = lines.find(line);
        if (existing != lines.end() && existing->second.mode == mode && existing->second.pullUp == pullUp) {
            if (mode == RequestMode::Output && gpiod_line_set_value(existing->second.line, initialState ? 1 : 0) < 0) {
                throw systemError("gpiod_line_set_value");
            }
            return;
        }

        if (existing != lines.end()) {
            gpiod_line_release(existing->second.line);
            lines.erase(existing);
        }

        int flags = pullUp ? GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP : 0;
        gpiod_line *handle = gpiod_chip_get_line(chip, line);
        if (handle == nullptr) {
            throw systemError("gpiod_chip_get_line");
        }

        int result = -1;
        switch (mode) {
            case RequestMode::Output:
                result = gpiod_line_request_output_flags(handle, "inky", flags, initialState ? 1 : 0);
                break;
            case RequestMode::Input:
                result = gpiod_line_request_input_flags(handle, "inky", flags);
                break;
            case RequestMode::FallingEdgeInput:
                result = gpiod_line_request_falling_edge_events_flags(handle, "inky", flags);
                break;
        }
        if (result < 0) {
            throw systemError("gpiod_line_request");
        }

        lines[line] = LineHandle{handle, mode, pullUp};
    }

    gpiod_chip *chip{nullptr};
    std::unordered_map<unsigned int, LineHandle> lines;
};

LinuxGpio::LinuxGpio(const std::string &chipPath) : impl_(std::make_unique<Impl>(chipPath)) {}
LinuxGpio::~LinuxGpio() = default;

void LinuxGpio::configureOutput(unsigned int line, bool initialState) {
    impl_->requestLine(line, Impl::RequestMode::Output, initialState, false);
}

void LinuxGpio::configureInput(unsigned int line, bool pullUp, GpioEdge edge) {
    const auto mode = edge == GpioEdge::Falling ? Impl::RequestMode::FallingEdgeInput : Impl::RequestMode::Input;
    impl_->requestLine(line, mode, false, pullUp);
}

void LinuxGpio::setValue(unsigned int line, bool active) {
    auto it = impl_->lines.find(line);
    if (it == impl_->lines.end()) {
        throw std::runtime_error("GPIO line not requested");
    }
    if (gpiod_line_set_value(it->second.line, active ? 1 : 0) < 0) {
        throw systemError("gpiod_line_set_value");
    }
}

bool LinuxGpio::getValue(unsigned int line) const {
    auto it = impl_->lines.find(line);
    if (it == impl_->lines.end()) {
        throw std::runtime_error("GPIO line not requested");
    }
    int value = gpiod_line_get_value(it->second.line);
    if (value < 0) {
        throw systemError("gpiod_line_get_value");
    }
    return value != 0;
}

std::optional<unsigned int> LinuxGpio::waitForEdge(std::span<const unsigned int> lines, std::chrono::milliseconds timeout) {
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline) {
        for (unsigned int line : lines) {
            auto it = impl_->lines.find(line);
            if (it == impl_->lines.end()) {
                continue;
            }
            if (it->second.mode != Impl::RequestMode::FallingEdgeInput) {
                continue;
            }

            timespec ts{};
            ts.tv_sec = 0;
            ts.tv_nsec = 1'000'000;
            const int waitResult = gpiod_line_event_wait(it->second.line, &ts);
            if (waitResult < 0) {
                throw systemError("gpiod_line_event_wait");
            }
            if (waitResult == 0) {
                continue;
            }

            gpiod_line_event event{};
            if (gpiod_line_event_read(it->second.line, &event) < 0) {
                throw systemError("gpiod_line_event_read");
            }
            return line;
        }
    }
    return std::nullopt;
}

LinuxSpi::LinuxSpi(const std::string &device, std::uint32_t speed) : fd_(-1), speed_(speed) {
    fd_ = ::open(device.c_str(), O_RDWR);
    if (fd_ < 0) {
        throw systemError("open spidev");
    }

    std::uint8_t mode = SPI_MODE_0;
    std::uint8_t bits = 8;

    if (ioctl(fd_, SPI_IOC_WR_MODE, &mode) < 0 || ioctl(fd_, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
        throw systemError("configure spidev");
    }

    if (ioctl(fd_, SPI_IOC_WR_MAX_SPEED_HZ, &speed_) < 0) {
        throw systemError("set max speed");
    }
}

LinuxSpi::~LinuxSpi() {
    if (fd_ >= 0) {
        ::close(fd_);
    }
}

void LinuxSpi::transfer(std::span<const std::uint8_t> data) {
    constexpr std::size_t chunkSize = 4096;
    for (std::size_t offset = 0; offset < data.size(); offset += chunkSize) {
        std::size_t remaining = std::min(chunkSize, data.size() - offset);
        spi_ioc_transfer transfer{};
        transfer.tx_buf = reinterpret_cast<std::uintptr_t>(data.data() + offset);
        transfer.len = static_cast<std::uint32_t>(remaining);
        transfer.speed_hz = speed_;
        transfer.bits_per_word = 8;
        transfer.cs_change = 0;

        if (ioctl(fd_, SPI_IOC_MESSAGE(1), &transfer) < 0) {
            throw systemError("SPI transfer");
        }
    }
}
#endif

}  // namespace inky
