# PsyGater — Goa Classic Edition

A JUCE 8 Windows VST3 effect for classic Goa trance energy with warm analog movement, roomy delayed echoes, and bright sunrise textures.

## Product profile

- Stereo VST3 effect for Windows
- Tempo-aware gate engine tuned for classic Goa trance grooves and hypnotic motion
- Warm stereo delay with smooth feedback and flowing cross-echo texture
- Eight factory presets built for sunrise, temple, and trance-festival energy
- Persistent host state and a clean architecture ready for refinement and expansion

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

- Preset: Sunrise Drift, Temple Lift, Golden Echo, Azure Vibe, Phantom Orbit, Maya Glide, Solar Pulse, Goa Bloom
- Gate Pattern: Straight, Goa, Offbeat, Stutter
- Depth / Width / Attack / Release / Swing: shaping the rhythmic gate motion
- Delay Time / Feedback / Dry-Wet / Output: shaping the classic Goa delay and spaciousness

## Design direction

This edition follows a brighter, more classic Goa trance identity: warm, melodic, atmospheric, and uplifting, with a more festival-friendly visual language and a gentler sunrise palette.
