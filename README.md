# PsyGater

PsyGater is a Windows VST3 effect designed around a Goa/Psytrance-inspired rhythmic gate and a stereo delay. It is implemented as a JUCE 8 plugin and built as a strong starting point for a production-ready VST effect.

## Features

- Stereo VST3 effect for Windows
- Rhythmic gate with tempo-aware rhythmic modulation
- Stereo echo feedback texture for psytrance-style delay trails
- Factory presets: Goa Pulse, Psy Chop, Echo Ritual and Forest Flow
- Persistent parameter state through JUCE's audio parameter tree

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

Or target only VST3:

```powershell
cmake --build build --config Release --target PsyGater_VST3
```

The generated VST3 bundle is typically placed under:

```text
build\PsyGater_artefacts\Release\VST3\
```

Copy the `.vst3` bundle to:

```text
C:\Program Files\Common Files\VST3\
```

## Preset design

- Goa Pulse: more aggressive gate and punchy, shorter delay.
- Psy Chop: tighter rhythmic gating with faster movement.
- Echo Ritual: a wider, more spacious delay tail.
- Forest Flow: smoother and atmospheric, ideal for drifting psychedelic textures.

## Notes

- This repository uses JUCE 8 via CMake `FetchContent`.
- The code is intentionally structured as a strong base for further sound design polish, automation, and GUI refinement.
- The plugin is Windows-focused and intended to be adapted or expanded for a production workflow.
