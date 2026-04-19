#include "inky/e673.hpp"
#include "inky/hardware.hpp"
#include "test.hpp"

#include <memory>
#include <stdexcept>
#include <vector>

namespace {

void expectInvalidArgument(const std::function<void()> &fn) {
    try {
        fn();
    } catch (const std::invalid_argument &) {
        return;
    }
    throw std::runtime_error("Expected std::invalid_argument");
}

}  // namespace

INKY_TEST(e673_rejects_invalid_native_indices) {
    auto spi = std::make_shared<inky::NullSpi>();
    auto gpio = std::make_shared<inky::MemoryGpio>();
    inky::E673 display(spi, gpio);

    std::vector<std::uint8_t> pixels(inky::E673::Width * inky::E673::Height, 1);
    pixels[123] = 4;
    expectInvalidArgument([&] { display.setImageIndices(pixels); });
}

INKY_TEST(e673_accepts_valid_native_indices) {
    auto spi = std::make_shared<inky::NullSpi>();
    auto gpio = std::make_shared<inky::MemoryGpio>();
    inky::E673 display(spi, gpio);

    std::vector<std::uint8_t> pixels(inky::E673::Width * inky::E673::Height, 6);
    pixels[0] = 0;
    pixels[1] = 1;
    pixels[2] = 2;
    pixels[3] = 3;
    pixels[4] = 5;
    display.setImageIndices(pixels);

    INKY_ASSERT_EQ(display.buffer()[0], 0);
    INKY_ASSERT_EQ(display.buffer()[1], 1);
    INKY_ASSERT_EQ(display.buffer()[2], 2);
    INKY_ASSERT_EQ(display.buffer()[3], 3);
    INKY_ASSERT_EQ(display.buffer()[4], 5);
}

INKY_TEST(e673_quantizes_rgb_into_native_palette_indices) {
    auto spi = std::make_shared<inky::NullSpi>();
    auto gpio = std::make_shared<inky::MemoryGpio>();
    inky::E673 display(spi, gpio);

    std::vector<std::uint8_t> rgb(inky::E673::Width * inky::E673::Height * 3, 255);
    const std::vector<std::array<std::uint8_t, 3>> firstPixels = {
        {0, 0, 0},
        {255, 255, 255},
        {255, 255, 0},
        {255, 0, 0},
        {0, 0, 255},
        {0, 255, 0},
    };

    for (std::size_t i = 0; i < firstPixels.size(); ++i) {
        rgb[i * 3 + 0] = firstPixels[i][0];
        rgb[i * 3 + 1] = firstPixels[i][1];
        rgb[i * 3 + 2] = firstPixels[i][2];
    }

    display.setImageRgb(rgb, 0.0f);

    INKY_ASSERT_EQ(display.buffer()[0], 0);
    INKY_ASSERT_EQ(display.buffer()[1], 1);
    INKY_ASSERT_EQ(display.buffer()[2], 2);
    INKY_ASSERT_EQ(display.buffer()[3], 3);
    INKY_ASSERT_EQ(display.buffer()[4], 5);
    INKY_ASSERT_EQ(display.buffer()[5], 6);
}
