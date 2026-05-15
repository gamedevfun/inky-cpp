#include "image_loader.hpp"
#include "test.hpp"

#include <cstdint>
#include <cstdlib>
#include <functional>
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

void expectBytesNear(
    std::span<const std::uint8_t> actual,
    std::span<const std::uint8_t> expected,
    int tolerance) {
    if (actual.size() != expected.size()) {
        throw std::runtime_error("Expected byte vectors to have matching sizes");
    }

    for (std::size_t i = 0; i < actual.size(); ++i) {
        const int delta = std::abs(static_cast<int>(actual[i]) - static_cast<int>(expected[i]));
        if (delta > tolerance) {
            throw std::runtime_error("Expected resized bytes to stay within tolerance of Pillow reference");
        }
    }
}

}  // namespace

INKY_TEST(image_loader_rejects_zero_sized_images) {
    expectInvalidArgument([] {
        const std::vector<std::uint8_t> empty;
        static_cast<void>(inky::examples::resizeBicubicRgb(empty, 0, 1, 1, 1));
    });

    expectInvalidArgument([] {
        const std::vector<std::uint8_t> pixels(3, 0);
        static_cast<void>(inky::examples::resizeBicubicRgb(pixels, 1, 1, 0, 1));
    });
}

INKY_TEST(image_loader_matches_pillow_bicubic_resize_for_rgb_images) {
    const std::vector<std::uint8_t> source = {
        255, 0, 0,
        0, 255, 0,
        0, 0, 255,
        255, 255, 255,
    };

    const std::vector<std::uint8_t> expected = {
        255, 0, 0, 215, 53, 0, 40, 202, 0, 0, 255, 0,
        202, 0, 53, 171, 53, 53, 84, 202, 53, 53, 255, 53,
        53, 0, 202, 84, 53, 202, 171, 202, 202, 202, 255, 202,
        0, 0, 255, 40, 53, 255, 215, 202, 255, 255, 255, 255,
    };

    const auto resized = inky::examples::resizeBicubicRgb(source, 2, 2, 4, 4);
    INKY_ASSERT_EQ(resized.size(), expected.size());
    INKY_ASSERT_EQ(resized[0], expected[0]);
    INKY_ASSERT_EQ(resized[1], expected[1]);
    INKY_ASSERT_EQ(resized[2], expected[2]);
    INKY_ASSERT_EQ(resized[45], expected[45]);
    INKY_ASSERT_EQ(resized[46], expected[46]);
    INKY_ASSERT_EQ(resized[47], expected[47]);
    expectBytesNear(resized, expected, 16);
}
