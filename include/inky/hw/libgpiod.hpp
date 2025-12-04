#pragma once

#include "inky/gpio.hpp"

#ifdef INKY_ENABLE_LIBGPIOD
#include <gpiod.h>
#endif

#include <string>
#include <unordered_map>

namespace inky {

class LibgpiodGpio : public GpioInterface {
public:
    explicit LibgpiodGpio(std::string chip = "gpiochip0");
    ~LibgpiodGpio() override;

    void configure_output(int pin, bool initial_state) override;
    void configure_input(int pin, bool pull_up) override;
    void write(int pin, bool value) override;
    [[nodiscard]] bool read(int pin) const override;

private:
#ifdef INKY_ENABLE_LIBGPIOD
    gpiod_chip* chip_;
    mutable std::unordered_map<int, gpiod_line*> lines_;
#endif
    std::string chip_name_;
};

}  // namespace inky

