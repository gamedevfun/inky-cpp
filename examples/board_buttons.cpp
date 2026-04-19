#include "inky/board.hpp"

#include <chrono>
#include <iostream>
#include <memory>

namespace {
const char *toString(inky::Button button) {
    switch (button) {
        case inky::Button::A:
            return "A";
        case inky::Button::B:
            return "B";
        case inky::Button::C:
            return "C";
        case inky::Button::D:
            return "D";
    }
    return "?";
}
}  // namespace

int main() {
#ifdef INKY_ENABLE_LINUX_GPIOD
    auto gpio = std::make_shared<inky::LinuxGpio>();
    inky::Board board(gpio);

    std::cout << "Waiting up to 10 seconds for an Inky button press..." << std::endl;
    const auto button = board.waitForButton(std::chrono::seconds(10));
    if (!button.has_value()) {
        std::cout << "No button press detected." << std::endl;
        return 0;
    }

    std::cout << "Detected button " << toString(*button) << "." << std::endl;
#else
    auto gpio = std::make_shared<inky::MemoryGpio>();
    inky::Board board(gpio);
    board.isPressed(inky::Button::A);
    gpio->setValue(5, false);
    const auto button = board.waitForButton(std::chrono::milliseconds(10));
    std::cout << "Mock backend button result: " << (button.has_value() ? toString(*button) : "none") << std::endl;
#endif

    return 0;
}
