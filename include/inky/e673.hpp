#pragma once

#include "gpio.hpp"
#include "spi.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <vector>

namespace inky {

struct Pinout {
    int cs = 8;
    int dc = 22;
    int reset = 27;
    int busy = 17;
};

class InkyE673 {
public:
    enum class Color : std::uint8_t {
        Black = 0,
        White = 1,
        Yellow = 2,
        Red = 3,
        Blue = 5,
        Green = 6,
    };

    static constexpr std::size_t width = 800;
    static constexpr std::size_t height = 480;

    InkyE673(SpiPtr spi, GpioPtr gpio, Pinout pins = {}, bool h_flip = false, bool v_flip = false);

    void set_pixel(std::size_t x, std::size_t y, Color colour);
    void fill(Color colour);
    void set_image(std::span<const Color> image);
    void set_border(Color colour);
    void show(bool busy_wait = true);

private:
    static constexpr std::uint8_t EL673_PSR = 0x00;
    static constexpr std::uint8_t EL673_PWR = 0x01;
    static constexpr std::uint8_t EL673_POF = 0x02;
    static constexpr std::uint8_t EL673_POFS = 0x03;
    static constexpr std::uint8_t EL673_PON = 0x04;
    static constexpr std::uint8_t EL673_BTST1 = 0x05;
    static constexpr std::uint8_t EL673_BTST2 = 0x06;
    static constexpr std::uint8_t EL673_DSLP = 0x07;
    static constexpr std::uint8_t EL673_BTST3 = 0x08;
    static constexpr std::uint8_t EL673_DTM1 = 0x10;
    static constexpr std::uint8_t EL673_DSP = 0x11;
    static constexpr std::uint8_t EL673_DRF = 0x12;
    static constexpr std::uint8_t EL673_PLL = 0x30;
    static constexpr std::uint8_t EL673_CDI = 0x50;
    static constexpr std::uint8_t EL673_TCON = 0x60;
    static constexpr std::uint8_t EL673_TRES = 0x61;
    static constexpr std::uint8_t EL673_REV = 0x70;
    static constexpr std::uint8_t EL673_VDCS = 0x82;
    static constexpr std::uint8_t EL673_PWS = 0xE3;

    SpiPtr spi_;
    GpioPtr gpio_;
    Pinout pins_;
    bool h_flip_ = false;
    bool v_flip_ = false;
    Color border_ = Color::White;
    bool initialized_ = false;
    std::vector<std::uint8_t> buffer_;

    void initialize();
    void reset();
    void busy_wait(std::chrono::duration<double> timeout) const;
    void update(std::span<const std::uint8_t> buffer);
    void send_command(std::uint8_t command, std::span<const std::uint8_t> data = {});
    [[nodiscard]] std::vector<std::uint8_t> pack_buffer(std::vector<std::uint8_t> region) const;
};

}  // namespace inky

