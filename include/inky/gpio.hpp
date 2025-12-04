#pragma once

#include <cstdint>
#include <memory>

namespace inky {

class GpioInterface {
public:
    virtual ~GpioInterface() = default;

    virtual void configure_output(int pin, bool initial_state) = 0;
    virtual void configure_input(int pin, bool pull_up) = 0;
    virtual void write(int pin, bool value) = 0;
    [[nodiscard]] virtual bool read(int pin) const = 0;
};

class NullGpio : public GpioInterface {
public:
    void configure_output(int, bool) override {}
    void configure_input(int, bool) override {}
    void write(int, bool) override {}
    [[nodiscard]] bool read(int) const override { return true; }
};

using GpioPtr = std::unique_ptr<GpioInterface>;

}  // namespace inky
