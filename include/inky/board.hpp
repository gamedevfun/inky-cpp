#pragma once

#include "inky/hardware.hpp"

#include <array>
#include <chrono>
#include <memory>
#include <optional>

namespace inky {

enum class Button {
    A,
    B,
    C,
    D,
};

struct BoardPins {
    unsigned int led = 13;
    unsigned int buttonA = 5;
    unsigned int buttonB = 6;
    unsigned int buttonC = 16;
    unsigned int buttonD = 24;
};

class Board {
public:
    explicit Board(std::shared_ptr<GpioController> gpio, BoardPins pins = {});

    void setLed(bool on);
    bool led() const;

    bool isPressed(Button button) const;
    std::optional<Button> waitForButton(std::chrono::milliseconds timeout);

private:
    unsigned int lineFor(Button button) const;
    void ensureSetup();

    std::shared_ptr<GpioController> gpio_;
    BoardPins pins_;
    bool setup_{false};
};

}  // namespace inky
