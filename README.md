# PsyGater — Final Product Version

A JUCE 8 Windows VST3 effect built for Goa/Psytrance-inspired rhythmic gating and stereo delay.

## Product profile

- Stereo VST3 effect for Windows
- Tempo-aware gate engine with multiple rhythmic patterns
- Stereo delay with feedback and ping-pong-style response
- Eight factory presets tuned for psytrance and Goa aesthetics
- Host parameter persistence and clean plugin-state handling
- Production-ready code structure for later refinement or commercial packaging

## Build requirements

- Windows 10 or later
- Visual Studio 2022 with Desktop development with C++
- CMake 3.22+
- Git

## Build

Open a Developer PowerShell for VS 2022 and run:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release --target PsyGater
```

The generated plugin bundle will typically appear under:

```text
build\PsyGater_artefacts\Release\VST3\
```

To install it manually:

```text
C:\Program Files\Common Files\VST3\
```

After copying, rescan your DAW plugin list.

## Controls

- Preset: eight starting-point sound designs
- Gate Pattern: Straight, Goa, Offbeat, Stutter
- Rate: tempo-synced note values or free-running Hz values
- Depth / Width / Attack / Release / Swing: shaping the gate motion
- Delay Time / Feedback / Dry-Wet / Output: shaping the stereo echo path

## Product notes

This version is a final, production-oriented working foundation for a Windows VST3 plugin. It is built around JUCE 8 and is suitable as a stable base for further sound design tuning, GUI polish, and DAW validation.
