# PsyGater

PsyGater is a Windows VST3 effect designed around a Goa/Psytrance-inspired rhythmic gate and a stereo delay. It is implemented as a JUCE 8 plugin and structured as a solid starting point for a more advanced production-grade effect.

## Features

- Stereo VST3 effect for Windows
- Rhythmic gate with tempo-aware pattern modulation
- Stereo delayed feedback texture for psychedelic echoes
- Factory presets: Goa Pulse, Psy Chop, Echo Ritual and Forest Flow
- Host-saving parameter state through JUCE's `AudioProcessorValueTreeState`

## Build instructions

### Requirements

- Windows 10 or later
- Visual Studio 2022 with Desktop development with C++
- CMake 3.22+
- Git

### Configure and build

Open a Developer PowerShell for VS 2022 and run:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

If you want only the VST3 target:

```powershell
cmake --build build --config Release --target PsyGater_VST3
```

The generated VST3 bundle is typically placed under:

```text
build\PsyGater_artefacts\Release\VST3\
```

You can then copy the generated `.vst3` bundle to:

```text
C:\Program Files\Common Files\VST3\
```

## Preset design

The factory presets are intentionally tuned toward rhythmic psytrance textures:

- Goa Pulse: wide gate + punchy short delay
- Psy Chop: fast, tight rhythmic gating
- Echo Ritual: deep delayed ambience with slower release
- Forest Flow: slower cycling, more spacious and atmospheric response

## Notes

- This repository uses JUCE 8 via CMake `FetchContent`.
- The plugin is intended as a strong starting point for further refinement in sound design, GUI polish and extra modulation features.
- The build is Windows-focused, but the code is still based on JUCE and can be adapted for other platforms with minimal changes.
