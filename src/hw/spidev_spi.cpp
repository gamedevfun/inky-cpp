#include "inky/hw/spidev.hpp"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <stdexcept>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

namespace inky {

namespace {
constexpr std::uint8_t spi_mode = SPI_MODE_0;
constexpr std::uint8_t bits_per_word = 8;
}

Spidev::Spidev(std::string device) : device_path_(std::move(device)) {}

Spidev::~Spidev() {
    if (fd_ >= 0) {
        close(fd_);
    }
}

void Spidev::set_max_speed_hz(std::uint32_t speed_hz) {
    speed_hz_ = speed_hz;
    if (fd_ >= 0) {
        ioctl(fd_, SPI_IOC_WR_MAX_SPEED_HZ, &speed_hz_);
    }
}

void Spidev::transfer(std::span<const std::uint8_t> values) {
    ensure_open();

    spi_ioc_transfer transfer{};
    transfer.tx_buf = reinterpret_cast<__u64>(values.data());
    transfer.rx_buf = 0;
    transfer.len = static_cast<__u32>(values.size());
    transfer.speed_hz = speed_hz_;
    transfer.bits_per_word = bits_per_word;

    int result = ioctl(fd_, SPI_IOC_MESSAGE(1), &transfer);
    if (result < 0) {
        throw std::runtime_error("SPI transfer failed: " + std::string(std::strerror(errno)));
    }
}

void Spidev::ensure_open() {
    if (fd_ >= 0) {
        return;
    }

    fd_ = open(device_path_.c_str(), O_RDWR);
    if (fd_ < 0) {
        throw std::runtime_error("Failed to open SPI device: " + device_path_);
    }

    if (ioctl(fd_, SPI_IOC_WR_MODE, &spi_mode) < 0) {
        throw std::runtime_error("Unable to configure SPI mode");
    }

    if (ioctl(fd_, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word) < 0) {
        throw std::runtime_error("Unable to configure SPI word size");
    }

    if (ioctl(fd_, SPI_IOC_WR_MAX_SPEED_HZ, &speed_hz_) < 0) {
        throw std::runtime_error("Unable to configure SPI speed");
    }
}

}  // namespace inky

