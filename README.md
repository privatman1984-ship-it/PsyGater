# PsyGater

PsyGater is a Windows VST3 audio effect for Goa and Psytrance production. It combines a tempo-aware rhythmic gate with a stereo ping-pong delay and four factory starting-point presets.

## Features

- VST3 effect built with JUCE 8 and CMake.
- Rhythmic gate with host-tempo divisions (1/4, 1/8, 1/16, 1/32) and free rates.
- Gate depth, pulse width, attack and release controls.
- Stereo ping-pong delay with time, feedback and wet-level controls.
- Factory presets: Goa Pulse, Psy Chop, Echo Ritual and Forest Flow.
- Plugin state is saved and recalled by the host.

> This is a starter implementation. Host tempo sync uses the host's reported BPM; the gate phase is free-running rather than locked to host transport position. Factory presets are parameter snapshots, not external `.vstpreset` files.

## Build on Windows

### Requirements

- Windows 10 or later
- Visual Studio 2022 with **Desktop development with C++** and a Windows 10/11 SDK
- CMake 3.22 or newer
- Git and an internet connection for the first configure (JUCE is fetched by CMake)

From a **Developer PowerShell for VS 2022** in the repository directory:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release --target PsyGater_VST3
```

The VST3 bundle is generated under `build\PsyGater_artefacts\Release\VST3\PsyGater.vst3`. With `COPY_PLUGIN_AFTER_BUILD`, JUCE also attempts to copy it to the standard VST3 folder. If not, copy the bundle to:

```text
C:\Program Files\Common Files\VST3\
```

Restart or rescan plugins in your DAW. If you use another Visual Studio generator, adjust the configuration and architecture options accordingly.

## Controls

- **Preset**: select one of four factory settings.
- **Depth**: amount of attenuation during the closed part of the gate pattern.
- **Rate**: host-synced rhythmic division or free-running frequency.
- **Width**: proportion of each cycle for which the gate is open.
- **Attack / Release**: smooth gate transitions in milliseconds.
- **Delay**: stereo delay time in milliseconds.
- **Feedback**: repeats fed back into the delay, internally limited below self-oscillation.
- **Delay Mix**: balance between gated signal and delayed signal.
- **Output**: overall dry-to-processed mix.

## Notes

The project fetches JUCE 8.0.8 from its upstream Git repository during CMake configuration. JUCE is licensed separately; review its terms before distributing binaries. The plugin uses JUCE's VST3 wrapper and does not include Steinberg SDK source.
