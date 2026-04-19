#include "inky/board.hpp"
#include "test.hpp"

#include <chrono>
#include <memory>

INKY_TEST(board_led_state_round_trips) {
    auto gpio = std::make_shared<inky::MemoryGpio>();
    inky::Board board(gpio);

    board.setLed(true);
    INKY_ASSERT(board.led());

    board.setLed(false);
    INKY_ASSERT(!board.led());
}

INKY_TEST(board_reads_button_state) {
    auto gpio = std::make_shared<inky::MemoryGpio>();
    inky::Board board(gpio);

    INKY_ASSERT(!board.isPressed(inky::Button::A));
    gpio->setValue(5, false);
    INKY_ASSERT(board.isPressed(inky::Button::A));
}

INKY_TEST(board_wait_for_button_reports_press) {
    auto gpio = std::make_shared<inky::MemoryGpio>();
    inky::Board board(gpio);

    board.isPressed(inky::Button::C);
    gpio->setValue(16, false);

    const auto button = board.waitForButton(std::chrono::milliseconds(10));
    INKY_ASSERT(button.has_value());
    INKY_ASSERT_EQ(*button, inky::Button::C);
}

INKY_TEST(board_wait_for_button_times_out) {
    auto gpio = std::make_shared<inky::MemoryGpio>();
    inky::Board board(gpio);

    const auto button = board.waitForButton(std::chrono::milliseconds(5));
    INKY_ASSERT(!button.has_value());
}
