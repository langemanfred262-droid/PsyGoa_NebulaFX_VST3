# PsyGoa NebulaFX VST3 — Version 2

Psytrance / Goatrance stereo multi-effect with a MIDI-playable FM lead voice.

## Version 2 features
- Tempo/transport-aware trance gate with selectable beat subdivision
- Resonant low-pass acid filter and LFO sweep
- Phaser and flanger modulation stages
- Beat-synchronised stereo ping-pong delay
- Algorithmic ambience/reverb
- MIDI-triggered 2-operator FM lead voice with ratio, index, envelope, and mix
- Host-automatable parameters and processor state save/restore
- 100 preset recipes plus VST3 preset-container files
- Dark neon UI concept artwork (`GUI_CONCEPT.svg` and PNG)
- GitHub Actions workflow and signal-flow diagram

## Build
Steinberg's VST3 SDK is intentionally not bundled. Initialize the SDK submodule:
```bash
git submodule update --init --recursive
cmake -S . -B build -DVST3_SDK_ROOT=$PWD/vst3sdk
cmake --build build --config Release
```
Then validate the generated bundle with the SDK's `validator` and test it in your DAW.

## Important
This is a source project / developer starter, not a precompiled or certified plugin. The included `GUI_CONCEPT` files are the visual design specification; the current VST3 controller exposes automatable host parameters but does not yet embed that artwork as a custom interactive editor. Preset files are generated using the VST3 preset container layout and should be validated with the Steinberg SDK and target DAWs. JSON recipes are included as portable editable backups.

The FM engine is a simple two-operator sine FM voice triggered by incoming MIDI notes. Audio effects process both the incoming stereo signal and the FM voice.
