#pragma once

#include <cstdint>
#include <chrono>
#include <deque>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace inky {

enum class GpioEdge {
    None,
    Falling,
};

/**
 * Simple SPI abstraction that mirrors the byte-wise transfers used by the
 * Python implementation. Implementations may batch transfers internally to
 * reduce system calls.
 */
class SpiDevice {
public:
    virtual ~SpiDevice() = default;
    virtual void transfer(std::span<const std::uint8_t> data) = 0;
};

/**
 * GPIO abstraction used by the driver to toggle control pins and read the busy
 * line. Implementations may choose to ignore configuration requests on
 * platforms that do not require explicit reservation of lines.
 */
class GpioController {
public:
    virtual ~GpioController() = default;

    virtual void configureOutput(unsigned int line, bool initialState) {
        (void)line;
        (void)initialState;
    }

    virtual void configureInput(unsigned int line, bool pullUp, GpioEdge edge = GpioEdge::None) {
        (void)line;
        (void)pullUp;
        (void)edge;
    }

    virtual void setValue(unsigned int line, bool active) = 0;
    virtual bool getValue(unsigned int line) const = 0;
    virtual std::optional<unsigned int> waitForEdge(std::span<const unsigned int> lines, std::chrono::milliseconds timeout) {
        (void)lines;
        (void)timeout;
        return std::nullopt;
    }
};

class NullSpi : public SpiDevice {
public:
    void transfer(std::span<const std::uint8_t> /*data*/) override {}
};

class MemoryGpio : public GpioController {
public:
    explicit MemoryGpio(std::size_t lines = 64);

    void configureOutput(unsigned int line, bool initialState) override;
    void configureInput(unsigned int line, bool pullUp, GpioEdge edge = GpioEdge::None) override;
    void setValue(unsigned int line, bool active) override;
    bool getValue(unsigned int line) const override;
    std::optional<unsigned int> waitForEdge(std::span<const unsigned int> lines, std::chrono::milliseconds timeout) override;

private:
    void ensureSize(unsigned int line);
    struct LineConfig {
        bool isOutput{false};
        bool pullUp{false};
        GpioEdge edge{GpioEdge::None};
    };
    std::vector<bool> states_;
    std::vector<LineConfig> configs_;
    std::deque<unsigned int> edgeEvents_;
};

#ifdef INKY_ENABLE_LINUX_GPIOD
class LinuxGpio : public GpioController {
public:
    explicit LinuxGpio(const std::string &chipPath = "/dev/gpiochip0");
    ~LinuxGpio() override;

    void configureOutput(unsigned int line, bool initialState) override;
    void configureInput(unsigned int line, bool pullUp, GpioEdge edge = GpioEdge::None) override;
    void setValue(unsigned int line, bool active) override;
    bool getValue(unsigned int line) const override;
    std::optional<unsigned int> waitForEdge(std::span<const unsigned int> lines, std::chrono::milliseconds timeout) override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

class LinuxSpi : public SpiDevice {
public:
    LinuxSpi(const std::string &device = "/dev/spidev0.0", std::uint32_t speed = 1'000'000);
    ~LinuxSpi() override;

    void transfer(std::span<const std::uint8_t> data) override;

private:
    int fd_;
    std::uint32_t speed_;
};
#endif

}  // namespace inky
