#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <vector>

namespace inky::examples {

struct LoadedImage {
    std::size_t width{};
    std::size_t height{};
    std::vector<std::uint8_t> rgb;
};

LoadedImage loadImage(const std::filesystem::path &path);
// Example-only resizer that approximates Pillow's default bicubic RGB resize
// closely enough for local example parity checks, but is not intended to be a
// byte-for-byte reimplementation of Pillow internals.
std::vector<std::uint8_t> resizeBicubicRgb(
    std::span<const std::uint8_t> source,
    std::size_t sourceWidth,
    std::size_t sourceHeight,
    std::size_t destinationWidth,
    std::size_t destinationHeight);

}  // namespace inky::examples
