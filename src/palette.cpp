#include "palette.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace inky::palette {

std::array<std::uint8_t, 18> paletteFromSaturation(float saturation) {
    const float clamped = std::clamp(saturation, 0.0f, 1.0f);
    std::array<std::uint8_t, 18> palette{};

    for (std::size_t i = 0; i < kDesaturatedPalette.size(); ++i) {
        const float rs = static_cast<float>(kSaturatedPalette[i][0]) * clamped;
        const float gs = static_cast<float>(kSaturatedPalette[i][1]) * clamped;
        const float bs = static_cast<float>(kSaturatedPalette[i][2]) * clamped;

        const float rd = static_cast<float>(kDesaturatedPalette[i][0]) * (1.0f - clamped);
        const float gd = static_cast<float>(kDesaturatedPalette[i][1]) * (1.0f - clamped);
        const float bd = static_cast<float>(kDesaturatedPalette[i][2]) * (1.0f - clamped);

        palette[i * 3 + 0] = static_cast<std::uint8_t>(rs + rd);
        palette[i * 3 + 1] = static_cast<std::uint8_t>(gs + gd);
        palette[i * 3 + 2] = static_cast<std::uint8_t>(bs + bd);
    }

    return palette;
}

std::uint8_t quantizePixel(const std::array<std::uint8_t, 3> &pixel, std::span<const std::uint8_t> paletteBytes) {
    if (paletteBytes.size() != kDesaturatedPalette.size() * 3) {
        throw std::invalid_argument("Palette must contain exactly six RGB entries");
    }

    std::size_t bestIndex = 0;
    std::uint32_t bestDistance = std::numeric_limits<std::uint32_t>::max();

    for (std::size_t i = 0; i < kDesaturatedPalette.size(); ++i) {
        const std::uint8_t pr = paletteBytes[i * 3 + 0];
        const std::uint8_t pg = paletteBytes[i * 3 + 1];
        const std::uint8_t pb = paletteBytes[i * 3 + 2];

        const auto dr = static_cast<int>(pixel[0]) - static_cast<int>(pr);
        const auto dg = static_cast<int>(pixel[1]) - static_cast<int>(pg);
        const auto db = static_cast<int>(pixel[2]) - static_cast<int>(pb);

        const std::uint32_t distance = static_cast<std::uint32_t>(dr * dr + dg * dg + db * db);
        if (distance < bestDistance) {
            bestDistance = distance;
            bestIndex = i;
        }
    }

    return kPaletteRemap[bestIndex];
}

bool isValidNativeIndex(std::uint8_t index) {
    return std::find(kPaletteRemap.begin(), kPaletteRemap.end(), index) != kPaletteRemap.end();
}

std::vector<std::uint8_t> packNativeIndices(std::span<const std::uint8_t> indices) {
    if ((indices.size() % 2U) != 0U) {
        throw std::invalid_argument("Packed display buffers require an even number of pixels");
    }

    std::vector<std::uint8_t> packed(indices.size() / 2, 0);
    std::size_t outIndex = 0;
    for (std::size_t i = 0; i < indices.size(); i += 2) {
        packed[outIndex++] = static_cast<std::uint8_t>((indices[i] << 4) | (indices[i + 1] & 0x0F));
    }
    return packed;
}

}  // namespace inky::palette
