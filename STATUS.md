# Weft — build status

Updated by whoever works on the project (human or agent). **Keep this file
current after every session** — last section first.

## Legend

- ✅ done · 🔨 in progress · ⏸ blocked (state why) · 📅 planned

## Current state (2026-10-08)

| Layer | Status |
|---|---|
| core: param/chain/config/osc/osc_io | ✅ builds, 26/26 test cases, 142/142 assertions |
| core: CMake + local build | ✅ (MSYS2 ucrt64 g++ 16.2, cmake 4.4, Ninja) |
| core: CI (GH Actions, 3-OS) | ✅ green 3/3 |
| host: JuceBackend (IPluginBackend over JUCE, VST3) | ✅ written, API-verified against JUCE 8.0.4 source |
| host: test VST3 plugin (WeftSmokePlugin) + smoke CLI | ✅ written (DryMix float 0..1 def 0.8; Mode choice Soft/Hard/Off) |
| host: CMake (JUCE 8.0.4 sha256-pinned, opt-in WEFT_BUILD_HOST) | ✅ pushed, not yet CI-verified |
| host: CI (3-OS matrix + smoke param assertions) | 🔨 run 37721075424 red 3/3 (host) — CI-env fixes pushed (MSVC / X11+FT deps / C-language), new run building |
| cli/ (offline audio→audio) | ⏸ not started |
| docs (README/CONFIG/OSC/ARCHITECTURE) | ✅ |
| logo + name (Weft) | ✅ (assets/logo/) |

## Log

- **2026-10-08 (3)** — CI run 37721075424: core green 3/3, host red 3/3.
  Diagnosed from full job logs (all 3 = CI environment issues, not code):
  (1) Windows picked MinGW; JUCE 8.0.4 removed MinGW support → Windows host
  job now uses Visual Studio 17 2022 / x64. (2) Linux: juceaide needs
  freetype/fontconfig/X11 headers → added 13 dev packages to apt deps.
  (3) macOS: root project() declared CXX only, JUCE's cmake needs C enabled
  (CMAKE_C_COMPILE_OBJECT) → project now LANGUAGES C CXX. Local sanity
  build re-green (26/26). Pushed; new CI run is the source of truth.

- **2026-10-08 (2)** — JUCE host layer written by assistant (not subagent — two
  research-only subagents had burned ~38 min on JUCE API investigation). All
  CMake/API facts verified directly against JUCE 8.0.4 source: juce_add_plugin
  topology (STATIC shared code + VST3 wrapper MODULE; shared code auto-links
  only juce_audio_plugin_client → modules linked explicitly),
  juce_generate_juce_header ordering (after juce_add_plugin), BusesProperties
  via constructor (no overridable method in JUCE 8), getParameters() returns
  const Array<AudioProcessorParameter*>&, getAllValueStrings() is const.
  Local sanity build green (core 26/26, host OFF). Pushed in 3 commits
  (3460a13, 0081059, 8f9305e); CI run 37721075424 building the first VST3
  on 3 OSes. Bounded CI-fix subagent to follow if red.
- **2026-10-08** — Project scaffolded. Core C++17 library written
  (param model, Chain + IPluginBackend, JSON config parse/dump/merge, OSC 1.0
  wire codec + `/chain` addressing, portable UDP transport). 26 targeted test
  cases / 142 assertions all passing locally. Docs + example config + CI
  workflow written. First public GitHub push in progress.
  - Known local quirk: a stale multi-config CMake cache from an early failed
    configure lived in `build/` for a while; clean reconfigure fixed it.
    (Superseded: build dir is regenerated in CI.)

## Conventions for agents working on this repo

1. **This file is the source of truth** for progress. Update the "Current
   state" table and append a dated log entry at the end of every session.
2. Commit every substantial change with a clear message; no giant squashed
   commits.
3. Tests must stay targeted and meaningful — do NOT add filler tests to look
   productive. Keep `ctest` green before committing test-affecting changes.
4. Core stays platform-agnostic C++17, MIT, no plugin SDK dependencies.
   Plugin SDK work happens in `host/` (JUCE) only.
5. Never commit secrets. Never push to anything but this repo's `main`.
6. If blocked, record it in the table with the reason — do not silently stop.
