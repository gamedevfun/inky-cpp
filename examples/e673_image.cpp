#include "image_loader.hpp"
#include "inky/e673.hpp"
#include "inky/hardware.hpp"

#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

namespace {

void printUsage(const char *program) {
    std::cout << "Usage:\n    " << program << " --file image.jpg (--saturation 0.5)\n";
}

std::optional<std::filesystem::path> parsePathArgument(int &index, int argc, char *argv[]) {
    if (index + 1 >= argc) {
        throw std::invalid_argument("Missing value for --file");
    }
    ++index;
    return std::filesystem::path(argv[index]);
}

float parseFloatArgument(int &index, int argc, char *argv[]) {
    if (index + 1 >= argc) {
        throw std::invalid_argument("Missing value for --saturation");
    }
    ++index;
    std::size_t parsed = 0;
    const std::string value = argv[index];
    const float saturation = std::stof(value, &parsed);
    if (parsed != value.size()) {
        throw std::invalid_argument("Invalid --saturation value");
    }
    return saturation;
}

}  // namespace

int main(int argc, char *argv[]) {
    try {
        std::optional<std::filesystem::path> filePath;
        float saturation = 0.5f;

        for (int i = 1; i < argc; ++i) {
            const std::string argument = argv[i];
            if (argument == "--help" || argument == "-h") {
                printUsage(argv[0]);
                return 0;
            }
            if (argument == "--file" || argument == "-f") {
                filePath = parsePathArgument(i, argc, argv);
                continue;
            }
            if (argument == "--saturation" || argument == "-s") {
                saturation = parseFloatArgument(i, argc, argv);
                continue;
            }

            throw std::invalid_argument("Unknown argument: " + argument);
        }

        if (!filePath.has_value()) {
            printUsage(argv[0]);
            return 1;
        }

#ifdef INKY_ENABLE_LINUX_GPIOD
        auto gpio = std::make_shared<inky::LinuxGpio>();
        auto spi = std::make_shared<inky::LinuxSpi>();
#else
        inky::Pins pins{};
        auto gpio = std::make_shared<inky::MemoryGpio>();
        auto spi = std::make_shared<inky::NullSpi>();
        gpio->configureInput(pins.busy, true);
#endif

        inky::E673 display(spi, gpio);
        const auto loaded = inky::examples::loadImage(*filePath);
        const auto resized = inky::examples::resizeBicubicRgb(
            loaded.rgb,
            loaded.width,
            loaded.height,
            inky::E673::Width,
            inky::E673::Height);

        display.setImageRgb(resized, saturation);
        display.show(false);

        std::cout << "Displayed " << filePath->string() << " on Inky Impression 7.3." << std::endl;
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << std::endl;
        return 1;
    }
}
