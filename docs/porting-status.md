# Porting Status Tracker

Use this file to track what has been ported from Python and what still needs verification.

## Port Baseline

- Tracking start commit: `e4023d974e944da2cd34cf1a15464d8405ebcd6d`
- Commit date: `2025-11-24 12:27:07 UTC` (`2025-11-24 13:27 CET`)
- Commit message: `imports sort for linting`

## Status Legend

- `todo`: not ported yet
- `in_progress`: implementation in progress
- `ported`: implemented in C++
- `verified`: behavior parity verified by tests
- `blocked`: cannot proceed until dependency/decision is resolved

## Tracking Table

| Area | Python reference | C++ location | Status | Test coverage | Notes |
|---|---|---|---|---|---|
| E673 init/register sequence | `reference/python/driver/inky/inky_e673.py` | `src/e673.cpp` | in_progress | partial | Validate full init sequence parity. |
| Palette/index mapping | `reference/python/driver/inky/inky_e673.py` | `src/palette.cpp`, `src/e673.cpp` | verified | covered | Saturation clamp, remap, RGB quantization, and packed nibbles covered by C++ tests. |
| Border handling | `reference/python/driver/inky/inky_e673.py` | `src/e673.cpp` | ported | missing | Add dedicated border behavior tests. |
| Linux GPIO backend | `reference/python/driver/inky/inky.py` | `src/hardware.cpp` | ported | partial | Board edge-wait API added; target hardware validation still needed. |
| Linux SPI transfer behavior | `reference/python/driver/inky/inky.py` | `src/hardware.cpp` | ported | partial | Validate chunking/transfer options under load. |
| Spectra 6 LED example | `reference/python/examples/spectra6/led.py` | `examples/board_led.cpp`, `src/board.cpp` | ported | covered | Board helper wraps the LED pin behind the GPIO abstraction. |
| Spectra 6 button example | `reference/python/examples/spectra6/buttons.py` | `examples/board_buttons.cpp`, `src/board.cpp` | ported | covered | Falling-edge wait path covered by `MemoryGpio` tests; Linux runtime validation still needed. |
| Example parity behavior | `reference/python/examples/spectra6/*.py` | `examples/e673_show.cpp`, `examples/board_*.cpp` | ported | n/a | C++ examples now cover display, LED, and button flows. |

## Exit Criteria (Remove Python Reference)

1. All rows are at least `ported`.
2. Core behavior rows are marked `verified`.
3. CI passes without requiring Python reference files.
