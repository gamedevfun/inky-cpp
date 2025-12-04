#pragma once

#include "inky/spi.hpp"

#include <string>

namespace inky {

class Spidev : public SpiInterface {
public:
    Spidev(std::string device = "/dev/spidev0.0");
    ~Spidev() override;

    void set_max_speed_hz(std::uint32_t speed_hz) override;
    void transfer(std::span<const std::uint8_t> values) override;

private:
    std::string device_path_;
    int fd_ = -1;
    std::uint32_t speed_hz_ = 1'000'000;

    void ensure_open();
};

}  // namespace inky

