#include "inky/board.hpp"

#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

int main() {
#ifdef INKY_ENABLE_LINUX_GPIOD
    auto gpio = std::make_shared<inky::LinuxGpio>();
#else
    auto gpio = std::make_shared<inky::MemoryGpio>();
#endif

    inky::Board board(gpio);

    std::cout << "Blinking Inky LED three times." << std::endl;
    for (int i = 0; i < 3; ++i) {
        board.setLed(true);
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        board.setLed(false);
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    return 0;
}
