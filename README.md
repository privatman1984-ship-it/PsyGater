# PsyGater — Dark Ritual Edition

A JUCE 8 Windows VST3 effect for darker Goa/Psytrance settings with a more ritualistic, shadowy stereo delay character.

## Product profile

- Stereo VST3 effect for Windows
- Tempo-aware gate engine tuned for darker ritual sweeps and strobing movement
- Stereo delay with dense feedback and hypnotic cross-echo behavior
- Eight factory presets designed for deep Goa, ritual trance, and night-drive textures
- Persistent host state and clean plugin architecture for future refinement

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

The generated plugin bundle usually appears under:

```text
build\PsyGater_artefacts\Release\VST3\
```

Copy it to:

```text
C:\Program Files\Common Files\VST3\
```

## Controls

- Preset: Night Ritual, Sub Paradox, Temple Echo, Forest Vow, Afterglow Pulse, Glass Invocation, Deep Ember, Shadow Stutter
- Gate Pattern: Straight, Goa, Offbeat, Stutter
- Depth / Width / Attack / Release / Swing: shaping the rhythmic gate motion
- Delay Time / Feedback / Dry-Wet / Output: shaping the shadowy echo structure

## Design direction

This edition leans toward a darker, more ceremonial psycho-techno sound with a deeper visual identity and a more dramatic, ritualized tone palette.
