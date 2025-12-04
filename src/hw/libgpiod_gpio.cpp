#include "inky/hw/libgpiod.hpp"

#ifdef INKY_ENABLE_LIBGPIOD

#include <stdexcept>
#include <string>

namespace inky {

LibgpiodGpio::LibgpiodGpio(std::string chip) : chip_name_(std::move(chip)) {
    chip_ = gpiod_chip_open_by_name(chip_name_.c_str());
    if (chip_ == nullptr) {
        throw std::runtime_error("Unable to open GPIO chip: " + chip_name_);
    }
}

LibgpiodGpio::~LibgpiodGpio() {
    for (auto& entry : lines_) {
        if (entry.second != nullptr) {
            gpiod_line_release(entry.second);
        }
    }
    if (chip_ != nullptr) {
        gpiod_chip_close(chip_);
    }
}

void LibgpiodGpio::configure_output(int pin, bool initial_state) {
    gpiod_line* line = gpiod_chip_get_line(chip_, pin);
    if (line == nullptr) {
        throw std::runtime_error("Failed to acquire line " + std::to_string(pin));
    }

    int result = gpiod_line_request_output(line, "inky", initial_state ? 1 : 0);
    if (result < 0) {
        throw std::runtime_error("Failed to request output for line " + std::to_string(pin));
    }

    lines_[pin] = line;
}

void LibgpiodGpio::configure_input(int pin, bool pull_up) {
    gpiod_line* line = gpiod_chip_get_line(chip_, pin);
    if (line == nullptr) {
        throw std::runtime_error("Failed to acquire line " + std::to_string(pin));
    }

    int flags = pull_up ? GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP : GPIOD_LINE_REQUEST_FLAG_BIAS_DISABLE;
    int result = gpiod_line_request_input_flags(line, "inky", flags);
    if (result < 0) {
        throw std::runtime_error("Failed to request input for line " + std::to_string(pin));
    }

    lines_[pin] = line;
}

void LibgpiodGpio::write(int pin, bool value) {
    auto it = lines_.find(pin);
    if (it == lines_.end()) {
        throw std::runtime_error("GPIO line not configured: " + std::to_string(pin));
    }

    int result = gpiod_line_set_value(it->second, value ? 1 : 0);
    if (result < 0) {
        throw std::runtime_error("Failed to set value for line " + std::to_string(pin));
    }
}

bool LibgpiodGpio::read(int pin) const {
    auto it = lines_.find(pin);
    if (it == lines_.end()) {
        throw std::runtime_error("GPIO line not configured: " + std::to_string(pin));
    }

    int value = gpiod_line_get_value(it->second);
    if (value < 0) {
        throw std::runtime_error("Failed to read value for line " + std::to_string(pin));
    }

    return value != 0;
}

}  // namespace inky

#endif

