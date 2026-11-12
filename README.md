# PsyGater — Version 4

A JUCE 8 Windows VST3 audio effect for Goa/Psytrance-style rhythmic gating and ping-pong delay.

## Version 4 highlights

- Eight factory starting-point presets, including Goa Pulse, Psy Chop, Echo Ritual and Forest Flow.
- Four 16-cycle gate patterns: Straight, Goa, Offbeat and Stutter.
- Tempo-aware quarter, eighth, sixteenth and thirty-second rate settings, plus 4/8/12 Hz free rates.
- Adjustable gate depth, width, attack, release and swing.
- Stereo cross-feedback delay with bounded feedback and a shared dry/wet blend.
- Host parameter attachments and state persistence; presets recall their parameter values.

## Windows build

Requirements: Windows 10+, Visual Studio 2022 (Desktop development with C++), CMake 3.22+ and Git.

In Developer PowerShell for VS 2022:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release --target PsyGater_VST3
```

JUCE is fetched by CMake on first configure. The resulting bundle is usually under `build\PsyGater_artefacts\Release\VST3\`. Copy `PsyGater.vst3` to `C:\Program Files\Common Files\VST3\` if automatic installation is unavailable, then rescan plugins in your DAW.

## Controls

- **Preset** loads one of eight factory settings.
- **Gate Pattern** chooses Straight, Goa, Offbeat or Stutter.
- **Rate** selects tempo-synced note lengths or a free-running frequency.
- **Depth / Width / Attack / Release / Swing** shape the gate motion.
- **Delay Time / Feedback / Dry-Wet** control the stereo echo.
- **Output** blends the processed signal with the original input.

The plugin follows the host's reported BPM for its synced rates. Gate phase is free-running and is not reset to transport/PPQ position. The presets are factory parameter snapshots, not exported `.vstpreset` files. Please audition levels carefully when using high feedback or deep gating. JUCE is licensed separately; review its terms before redistributing binaries.
