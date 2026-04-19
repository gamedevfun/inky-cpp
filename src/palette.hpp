#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <vector>

namespace inky::palette {

inline constexpr std::array<std::array<std::uint8_t, 3>, 6> kDesaturatedPalette = {{{0, 0, 0}, {255, 255, 255}, {255, 255, 0}, {255, 0, 0}, {0, 0, 255}, {0, 255, 0}}};
inline constexpr std::array<std::array<std::uint8_t, 3>, 6> kSaturatedPalette = {{{0, 0, 0}, {161, 164, 165}, {208, 190, 71}, {156, 72, 75}, {61, 59, 94}, {58, 91, 70}}};
inline constexpr std::array<std::uint8_t, 6> kPaletteRemap = {0, 1, 2, 3, 5, 6};

std::array<std::uint8_t, 18> paletteFromSaturation(float saturation);
std::uint8_t quantizePixel(const std::array<std::uint8_t, 3> &pixel, std::span<const std::uint8_t> paletteBytes);
bool isValidNativeIndex(std::uint8_t index);
std::vector<std::uint8_t> packNativeIndices(std::span<const std::uint8_t> indices);

}  // namespace inky::palette
