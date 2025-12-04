#include "inky/e673.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <stdexcept>
#include <thread>

namespace inky {

namespace {
constexpr auto reset_delay = std::chrono::milliseconds(30);
constexpr auto busy_poll_interval = std::chrono::milliseconds(100);
}  // namespace

InkyE673::InkyE673(SpiPtr spi, GpioPtr gpio, Pinout pins, bool h_flip, bool v_flip)
    : spi_(std::move(spi)), gpio_(std::move(gpio)), pins_(pins), h_flip_(h_flip), v_flip_(v_flip) {
    if (spi_ == nullptr || gpio_ == nullptr) {
        throw std::invalid_argument("InkyE673 requires SPI and GPIO implementations");
    }

    buffer_.resize(width * height, static_cast<std::uint8_t>(Color::White));
}

void InkyE673::set_pixel(std::size_t x, std::size_t y, Color colour) {
    if (x >= width || y >= height) {
        throw std::out_of_range("Pixel coordinate outside display bounds");
    }

    buffer_[y * width + x] = static_cast<std::uint8_t>(colour) & 0x07;
}

void InkyE673::fill(Color colour) {
    std::fill(buffer_.begin(), buffer_.end(), static_cast<std::uint8_t>(colour) & 0x07);
}

void InkyE673::set_image(std::span<const Color> image) {
    if (image.size() != width * height) {
        throw std::invalid_argument("Image dimensions must match display resolution");
    }

    std::transform(image.begin(), image.end(), buffer_.begin(), [](Color colour) {
        return static_cast<std::uint8_t>(colour) & 0x07;
    });
}

void InkyE673::set_border(Color colour) {
    border_ = colour;
}

void InkyE673::show(bool) {
    initialize();

    std::vector<std::uint8_t> region;
    region.reserve(buffer_.size());

    for (std::size_t y = 0; y < height; ++y) {
        for (std::size_t x = 0; x < width; ++x) {
            std::size_t tx = h_flip_ ? (width - 1 - x) : x;
            std::size_t ty = v_flip_ ? (height - 1 - y) : y;
            region.push_back(buffer_[ty * width + tx]);
        }
    }

    auto packed = pack_buffer(std::move(region));
    update(packed);
}

void InkyE673::initialize() {
    if (initialized_) {
        return;
    }

    gpio_->configure_output(pins_.cs, true);
    gpio_->configure_output(pins_.dc, false);
    gpio_->configure_output(pins_.reset, true);
    gpio_->configure_input(pins_.busy, true);

    spi_->set_max_speed_hz(1'000'000);

    reset();
    busy_wait(std::chrono::milliseconds(300));

    const std::array<std::uint8_t, 6> bootstrap{0x49, 0x55, 0x20, 0x08, 0x09, 0x18};
    send_command(0xAA, bootstrap);
    send_command(EL673_PWR, std::array<std::uint8_t, 1>{0x3F});
    send_command(EL673_PSR, std::array<std::uint8_t, 2>{0x5F, 0x69});

    send_command(EL673_BTST1, std::array<std::uint8_t, 4>{0x40, 0x1F, 0x1F, 0x2C});
    send_command(EL673_BTST3, std::array<std::uint8_t, 4>{0x6F, 0x1F, 0x1F, 0x22});
    send_command(EL673_BTST2, std::array<std::uint8_t, 4>{0x6F, 0x1F, 0x17, 0x17});

    send_command(EL673_POFS, std::array<std::uint8_t, 4>{0x00, 0x54, 0x00, 0x44});
    send_command(EL673_TCON, std::array<std::uint8_t, 2>{0x02, 0x00});
    send_command(EL673_PLL, std::array<std::uint8_t, 1>{0x08});
    send_command(EL673_CDI, std::array<std::uint8_t, 1>{0x3F});
    send_command(EL673_TRES, std::array<std::uint8_t, 4>{0x03, 0x20, 0x01, 0xE0});
    send_command(EL673_PWS, std::array<std::uint8_t, 1>{0x2F});
    send_command(EL673_VDCS, std::array<std::uint8_t, 1>{0x01});

    initialized_ = true;
}

void InkyE673::reset() {
    gpio_->write(pins_.reset, false);
    std::this_thread::sleep_for(reset_delay);
    gpio_->write(pins_.reset, true);
    std::this_thread::sleep_for(reset_delay);
}

void InkyE673::busy_wait(std::chrono::duration<double> timeout) const {
    auto start = std::chrono::steady_clock::now();

    if (gpio_->read(pins_.busy)) {
        std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::milliseconds>(timeout));
        return;
    }

    while (!gpio_->read(pins_.busy)) {
        std::this_thread::sleep_for(busy_poll_interval);
        if (std::chrono::steady_clock::now() - start > timeout) {
            return;
        }
    }
}

void InkyE673::update(std::span<const std::uint8_t> buffer) {
    send_command(EL673_DTM1, buffer);
    send_command(EL673_PON);
    busy_wait(std::chrono::milliseconds(300));

    send_command(EL673_BTST2, std::array<std::uint8_t, 4>{0x6F, 0x1F, 0x17, 0x49});

    send_command(EL673_DRF, std::array<std::uint8_t, 1>{0x00});
    busy_wait(std::chrono::seconds(32));

    send_command(EL673_POF, std::array<std::uint8_t, 1>{0x00});
    busy_wait(std::chrono::milliseconds(300));
}

void InkyE673::send_command(std::uint8_t command, std::span<const std::uint8_t> data) {
    gpio_->write(pins_.cs, false);
    gpio_->write(pins_.dc, false);

    const std::array<std::uint8_t, 1> prefix{command};
    spi_->transfer(prefix);

    if (!data.empty()) {
        gpio_->write(pins_.dc, true);
        spi_->transfer(data);
    }

    gpio_->write(pins_.cs, true);
    gpio_->write(pins_.dc, false);
}

std::vector<std::uint8_t> InkyE673::pack_buffer(std::vector<std::uint8_t> region) const {
    if (region.size() != width * height) {
        throw std::invalid_argument("Buffer size must match display resolution");
    }

    std::vector<std::uint8_t> packed;
    packed.reserve(region.size() / 2);

    for (std::size_t i = 0; i + 1 < region.size(); i += 2) {
        std::uint8_t upper = (region[i] & 0x0F) << 4;
        std::uint8_t lower = region[i + 1] & 0x0F;
        packed.push_back(static_cast<std::uint8_t>(upper | lower));
    }

    return packed;
}

}  // namespace inky

