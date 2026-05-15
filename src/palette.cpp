#include "palette.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace inky::palette {
namespace {

std::size_t nearestPaletteIndex(const std::array<float, 3> &pixel, std::span<const std::uint8_t> paletteBytes) {
    std::size_t bestIndex = 0;
    float bestDistance = std::numeric_limits<float>::max();

    for (std::size_t i = 0; i < kDesaturatedPalette.size(); ++i) {
        const float pr = static_cast<float>(paletteBytes[i * 3 + 0]);
        const float pg = static_cast<float>(paletteBytes[i * 3 + 1]);
        const float pb = static_cast<float>(paletteBytes[i * 3 + 2]);

        const float dr = pixel[0] - pr;
        const float dg = pixel[1] - pg;
        const float db = pixel[2] - pb;

        const float distance = (dr * dr) + (dg * dg) + (db * db);
        if (distance < bestDistance) {
            bestDistance = distance;
            bestIndex = i;
        }
    }

    return bestIndex;
}

void diffuseError(
    std::vector<float> &working,
    std::size_t width,
    std::size_t height,
    std::size_t x,
    std::size_t y,
    const std::array<float, 3> &error,
    long dx,
    long dy,
    float weight) {
    const long targetX = static_cast<long>(x) + dx;
    const long targetY = static_cast<long>(y) + dy;
    if (targetX < 0 || targetY < 0 ||
        targetX >= static_cast<long>(width) || targetY >= static_cast<long>(height)) {
        return;
    }

    const std::size_t offset = (static_cast<std::size_t>(targetY) * width + static_cast<std::size_t>(targetX)) * 3;
    working[offset + 0] += error[0] * weight;
    working[offset + 1] += error[1] * weight;
    working[offset + 2] += error[2] * weight;
}

}  // namespace

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

    const std::size_t bestIndex = nearestPaletteIndex(
        {static_cast<float>(pixel[0]), static_cast<float>(pixel[1]), static_cast<float>(pixel[2])},
        paletteBytes);

    return kPaletteRemap[bestIndex];
}

std::vector<std::uint8_t> quantizeImageRgb(
    std::span<const std::uint8_t> rgbPixels,
    std::size_t width,
    std::size_t height,
    std::span<const std::uint8_t> paletteBytes) {
    if (paletteBytes.size() != kDesaturatedPalette.size() * 3) {
        throw std::invalid_argument("Palette must contain exactly six RGB entries");
    }
    if (rgbPixels.size() != width * height * 3) {
        throw std::invalid_argument("RGB image must contain width * height * 3 bytes");
    }

    std::vector<float> working(rgbPixels.begin(), rgbPixels.end());
    std::vector<std::uint8_t> output(width * height, 0);

    for (std::size_t y = 0; y < height; ++y) {
        for (std::size_t x = 0; x < width; ++x) {
            const std::size_t offset = (y * width + x) * 3;
            const std::array<float, 3> pixel{
                std::clamp(working[offset + 0], 0.0f, 255.0f),
                std::clamp(working[offset + 1], 0.0f, 255.0f),
                std::clamp(working[offset + 2], 0.0f, 255.0f),
            };
            const std::size_t paletteIndex = nearestPaletteIndex(pixel, paletteBytes);
            output[y * width + x] = kPaletteRemap[paletteIndex];

            const std::array<float, 3> palettePixel{
                static_cast<float>(paletteBytes[paletteIndex * 3 + 0]),
                static_cast<float>(paletteBytes[paletteIndex * 3 + 1]),
                static_cast<float>(paletteBytes[paletteIndex * 3 + 2]),
            };
            const std::array<float, 3> error{
                pixel[0] - palettePixel[0],
                pixel[1] - palettePixel[1],
                pixel[2] - palettePixel[2],
            };

            diffuseError(working, width, height, x, y, error, 1, 0, 7.0f / 16.0f);
            diffuseError(working, width, height, x, y, error, -1, 1, 3.0f / 16.0f);
            diffuseError(working, width, height, x, y, error, 0, 1, 5.0f / 16.0f);
            diffuseError(working, width, height, x, y, error, 1, 1, 1.0f / 16.0f);
        }
    }

    return output;
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
