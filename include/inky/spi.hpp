#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

namespace inky {

class SpiInterface {
public:
    virtual ~SpiInterface() = default;

    virtual void set_max_speed_hz(std::uint32_t speed_hz) = 0;
    virtual void transfer(std::span<const std::uint8_t> values) = 0;
};

class NullSpi : public SpiInterface {
public:
    void set_max_speed_hz(std::uint32_t) override {}
    void transfer(std::span<const std::uint8_t>) override {}
};

using SpiPtr = std::unique_ptr<SpiInterface>;

}  // namespace inky
