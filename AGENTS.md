# AGENTS.md

## Purpose

`inky-cpp` is a modern C++20 port of Pimoroni's Inky library, currently focused on the Inky Impression 7.3" Spectra 6 display (`E673`) plus lightweight Raspberry Pi board helpers.

## Architecture

- `include/` and `src/` hold the production C++ library.
- `E673` owns display buffer management, palette quantization entrypoints, and panel update sequencing.
- `GpioController` and `SpiDevice` isolate hardware access from display logic.
- `Board` exposes onboard LED and button helpers without mixing board concerns into `E673`.
- `reference/python/` is pinned reference material only; it must not be used by the C++ build or runtime.

## Build And Test

Native build:

```bash
cmake --preset native-debug
cmake --build --preset native-debug
ctest --test-dir build --output-on-failure
```

Portable build without Linux GPIO backend:

```bash
cmake -S . -B build -DINKY_ENABLE_LINUX_GPIOD=OFF
cmake --build build
ctest --test-dir build --output-on-failure
```

## Cross-Compile

- Use `rpi-aarch64-release` or `rpi-armhf-release` from `CMakePresets.json`.
- Cross builds that require GPIO support need `libgpiod` headers and pkg-config metadata present in `.sysroot`.

## Porting Rules

- Match behavior against the pinned Python reference before marking parity work complete.
- Prefer small internal helpers plus tests over embedding translation logic directly in driver methods.
- Add or update C++ tests for any behavior copied from Python.
- Keep board-level pin defaults and helpers outside `E673`.
- Do not add file/image decoding dependencies unless the repo scope explicitly expands.

## Where To Put Things

- Public headers: `include/inky/`
- Internal helpers: `src/`
- C++ examples: `examples/`
- C++ tests: `tests/`
- Porting notes and status: `docs/`

## Current Non-Goals

- General image loading APIs or decoder integration
- Support for non-E673 display variants in the production C++ API
- Callback/threaded button frameworks beyond synchronous wait/read helpers
