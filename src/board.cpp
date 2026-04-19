#include "inky/board.hpp"

#include <stdexcept>

namespace inky {

Board::Board(std::shared_ptr<GpioController> gpio, BoardPins pins) : gpio_(std::move(gpio)), pins_(pins) {
    if (!gpio_) {
        throw std::invalid_argument("GPIO controller must not be null");
    }
}

void Board::setLed(bool on) {
    ensureSetup();
    gpio_->setValue(pins_.led, on);
}

bool Board::led() const {
    if (!setup_) {
        return false;
    }
    return gpio_->getValue(pins_.led);
}

bool Board::isPressed(Button button) const {
    if (!setup_) {
        const_cast<Board *>(this)->ensureSetup();
    }
    return !gpio_->getValue(lineFor(button));
}

std::optional<Button> Board::waitForButton(std::chrono::milliseconds timeout) {
    ensureSetup();
    const std::array<unsigned int, 4> lines = {pins_.buttonA, pins_.buttonB, pins_.buttonC, pins_.buttonD};
    const auto eventLine = gpio_->waitForEdge(lines, timeout);
    if (!eventLine.has_value()) {
        return std::nullopt;
    }

    if (*eventLine == pins_.buttonA) {
        return Button::A;
    }
    if (*eventLine == pins_.buttonB) {
        return Button::B;
    }
    if (*eventLine == pins_.buttonC) {
        return Button::C;
    }
    if (*eventLine == pins_.buttonD) {
        return Button::D;
    }
    return std::nullopt;
}

unsigned int Board::lineFor(Button button) const {
    switch (button) {
        case Button::A:
            return pins_.buttonA;
        case Button::B:
            return pins_.buttonB;
        case Button::C:
            return pins_.buttonC;
        case Button::D:
            return pins_.buttonD;
    }
    throw std::invalid_argument("Unknown button");
}

void Board::ensureSetup() {
    if (setup_) {
        return;
    }

    gpio_->configureOutput(pins_.led, false);
    gpio_->configureInput(pins_.buttonA, true, GpioEdge::Falling);
    gpio_->configureInput(pins_.buttonB, true, GpioEdge::Falling);
    gpio_->configureInput(pins_.buttonC, true, GpioEdge::Falling);
    gpio_->configureInput(pins_.buttonD, true, GpioEdge::Falling);
    setup_ = true;
}

}  // namespace inky
