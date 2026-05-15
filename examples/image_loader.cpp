#include "image_loader.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>

#define STBI_NO_HDR
#define STBI_NO_PNM
#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

namespace inky::examples {
namespace {

[[noreturn]] void throwImageError(const std::string &message) {
    throw std::runtime_error("Image load failed: " + message);
}

double bicubicWeight(double distance) {
    constexpr double a = -0.5;
    const double x = std::abs(distance);
    if (x <= 1.0) {
        return ((a + 2.0) * x * x * x) - ((a + 3.0) * x * x) + 1.0;
    }
    if (x < 2.0) {
        return (a * x * x * x) - (5.0 * a * x * x) + (8.0 * a * x) - (4.0 * a);
    }
    return 0.0;
}

std::size_t sourceIndex(std::size_t width, std::size_t x, std::size_t y) {
    return (y * width + x) * 3;
}

std::uint8_t clampToByte(double value) {
    return static_cast<std::uint8_t>(std::clamp(std::lround(value), 0L, 255L));
}

}  // namespace

LoadedImage loadImage(const std::filesystem::path &path) {
    int width = 0;
    int height = 0;
    int channels = 0;
    const std::string filename = path.string();
    std::unique_ptr<stbi_uc, decltype(&stbi_image_free)> pixels(
        stbi_load(filename.c_str(), &width, &height, &channels, 3),
        stbi_image_free);
    if (pixels == nullptr) {
        const char *reason = stbi_failure_reason();
        throwImageError(reason != nullptr ? reason : "unknown failure");
    }

    LoadedImage image;
    image.width = static_cast<std::size_t>(width);
    image.height = static_cast<std::size_t>(height);
    image.rgb.assign(pixels.get(), pixels.get() + image.width * image.height * 3);
    return image;
}

std::vector<std::uint8_t> resizeBicubicRgb(
    std::span<const std::uint8_t> source,
    std::size_t sourceWidth,
    std::size_t sourceHeight,
    std::size_t destinationWidth,
    std::size_t destinationHeight) {
    if (sourceWidth == 0 || sourceHeight == 0) {
        throw std::invalid_argument("Source image dimensions must be non-zero");
    }
    if (destinationWidth == 0 || destinationHeight == 0) {
        throw std::invalid_argument("Destination image dimensions must be non-zero");
    }
    if (source.size() != sourceWidth * sourceHeight * 3) {
        throw std::invalid_argument("Source image must contain sourceWidth * sourceHeight * 3 bytes");
    }

    // This is an example-local bicubic resizer for RGB images. It is tuned to
    // stay visually close to Pillow's default bicubic path for the example
    // flow, especially before palette quantization, but does not try to match
    // Pillow's internal edge handling and rounding byte-for-byte.
    std::vector<std::uint8_t> destination(destinationWidth * destinationHeight * 3);
    for (std::size_t y = 0; y < destinationHeight; ++y) {
        for (std::size_t x = 0; x < destinationWidth; ++x) {
            const double mappedX = ((static_cast<double>(x) + 0.5) * static_cast<double>(sourceWidth) /
                                    static_cast<double>(destinationWidth)) - 0.5;
            const double mappedY = ((static_cast<double>(y) + 0.5) * static_cast<double>(sourceHeight) /
                                    static_cast<double>(destinationHeight)) - 0.5;
            const long xBase = static_cast<long>(std::floor(mappedX));
            const long yBase = static_cast<long>(std::floor(mappedY));
            const std::size_t destinationOffset = sourceIndex(destinationWidth, x, y);

            std::array<double, 3> channels{0.0, 0.0, 0.0};
            for (long sampleY = yBase - 1; sampleY <= yBase + 2; ++sampleY) {
                const std::size_t clampedY = static_cast<std::size_t>(
                    std::clamp(sampleY, 0L, static_cast<long>(sourceHeight - 1)));
                const double yWeight = bicubicWeight(mappedY - static_cast<double>(sampleY));
                for (long sampleX = xBase - 1; sampleX <= xBase + 2; ++sampleX) {
                    const std::size_t clampedX = static_cast<std::size_t>(
                        std::clamp(sampleX, 0L, static_cast<long>(sourceWidth - 1)));
                    const double weight = yWeight * bicubicWeight(mappedX - static_cast<double>(sampleX));
                    const std::size_t sampleOffset = sourceIndex(sourceWidth, clampedX, clampedY);
                    channels[0] += weight * static_cast<double>(source[sampleOffset + 0]);
                    channels[1] += weight * static_cast<double>(source[sampleOffset + 1]);
                    channels[2] += weight * static_cast<double>(source[sampleOffset + 2]);
                }
            }

            destination[destinationOffset + 0] = clampToByte(channels[0]);
            destination[destinationOffset + 1] = clampToByte(channels[1]);
            destination[destinationOffset + 2] = clampToByte(channels[2]);
        }
    }

    return destination;
}

}  // namespace inky::examples
