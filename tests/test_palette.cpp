#include "palette.hpp"
#include "test.hpp"

#include <array>
#include <vector>

INKY_TEST(palette_clamps_low_saturation) {
    const auto palette = inky::palette::paletteFromSaturation(-1.0f);
    INKY_ASSERT_EQ(palette[0], 0);
    INKY_ASSERT_EQ(palette[1], 0);
    INKY_ASSERT_EQ(palette[2], 0);
    INKY_ASSERT_EQ(palette[3], 255);
    INKY_ASSERT_EQ(palette[4], 255);
    INKY_ASSERT_EQ(palette[5], 255);
}

INKY_TEST(palette_blends_mid_saturation) {
    const auto palette = inky::palette::paletteFromSaturation(0.5f);
    INKY_ASSERT_EQ(palette[3], 208);
    INKY_ASSERT_EQ(palette[4], 209);
    INKY_ASSERT_EQ(palette[5], 210);
    INKY_ASSERT_EQ(palette[12], 30);
    INKY_ASSERT_EQ(palette[13], 29);
    INKY_ASSERT_EQ(palette[14], 174);
}

INKY_TEST(palette_clamps_high_saturation) {
    const auto palette = inky::palette::paletteFromSaturation(3.0f);
    INKY_ASSERT_EQ(palette[3], 161);
    INKY_ASSERT_EQ(palette[4], 164);
    INKY_ASSERT_EQ(palette[5], 165);
    INKY_ASSERT_EQ(palette[15], 58);
    INKY_ASSERT_EQ(palette[16], 91);
    INKY_ASSERT_EQ(palette[17], 70);
}

INKY_TEST(quantize_exact_palette_colours) {
    const auto palette = inky::palette::paletteFromSaturation(0.0f);
    INKY_ASSERT_EQ(inky::palette::quantizePixel({0, 0, 0}, palette), 0);
    INKY_ASSERT_EQ(inky::palette::quantizePixel({255, 255, 255}, palette), 1);
    INKY_ASSERT_EQ(inky::palette::quantizePixel({255, 255, 0}, palette), 2);
    INKY_ASSERT_EQ(inky::palette::quantizePixel({255, 0, 0}, palette), 3);
    INKY_ASSERT_EQ(inky::palette::quantizePixel({0, 0, 255}, palette), 5);
    INKY_ASSERT_EQ(inky::palette::quantizePixel({0, 255, 0}, palette), 6);
}

INKY_TEST(quantize_prefers_nearest_colour) {
    const auto palette = inky::palette::paletteFromSaturation(1.0f);
    INKY_ASSERT_EQ(inky::palette::quantizePixel({65, 62, 100}, palette), 5);
    INKY_ASSERT_EQ(inky::palette::quantizePixel({150, 80, 78}, palette), 3);
}

INKY_TEST(quantize_image_matches_python_reference_case_one) {
    const auto palette = inky::palette::paletteFromSaturation(0.5f);
    const std::vector<std::uint8_t> rgb = {
        255, 0, 0, 220, 40, 40, 180, 70, 70, 140, 90, 90,
        0, 255, 0, 20, 220, 40, 60, 180, 60, 100, 140, 80,
        0, 0, 255, 40, 40, 220, 70, 70, 180, 90, 90, 140,
        255, 255, 0, 220, 220, 40, 180, 180, 70, 140, 140, 90,
    };
    const std::vector<std::uint8_t> expected = {
        3, 3, 3, 3,
        6, 6, 6, 6,
        5, 5, 5, 1,
        2, 2, 2, 5,
    };

    const auto quantized = inky::palette::quantizeImageRgb(rgb, 4, 4, palette);
    INKY_ASSERT_EQ(quantized, expected);
}

INKY_TEST(quantize_image_matches_python_reference_case_two) {
    const auto palette = inky::palette::paletteFromSaturation(0.5f);
    const std::vector<std::uint8_t> rgb = {
        10, 10, 10, 120, 120, 120, 240, 240, 240, 250, 240, 20,
        250, 20, 20, 40, 40, 180, 40, 150, 60, 150, 120, 70,
        20, 20, 240, 220, 220, 120, 170, 80, 85, 90, 110, 150,
        5, 250, 5, 200, 200, 200, 100, 60, 60, 60, 100, 60,
    };
    const std::vector<std::uint8_t> expected = {
        0, 6, 1, 2,
        3, 5, 6, 3,
        5, 1, 3, 1,
        6, 1, 6, 5,
    };

    const auto quantized = inky::palette::quantizeImageRgb(rgb, 4, 4, palette);
    INKY_ASSERT_EQ(quantized, expected);
}

INKY_TEST(native_index_validation_and_packing) {
    INKY_ASSERT(inky::palette::isValidNativeIndex(0));
    INKY_ASSERT(inky::palette::isValidNativeIndex(6));
    INKY_ASSERT(!inky::palette::isValidNativeIndex(4));
    INKY_ASSERT(!inky::palette::isValidNativeIndex(255));

    const std::vector<std::uint8_t> indices = {0, 1, 2, 3, 5, 6};
    const auto packed = inky::palette::packNativeIndices(indices);
    INKY_ASSERT_EQ(packed.size(), std::size_t{3});
    INKY_ASSERT_EQ(packed[0], 0x01);
    INKY_ASSERT_EQ(packed[1], 0x23);
    INKY_ASSERT_EQ(packed[2], 0x56);
}
