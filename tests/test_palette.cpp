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
