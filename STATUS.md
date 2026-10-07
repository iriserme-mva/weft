# Weft — build status

Updated by whoever works on the project (human or agent). **Keep this file
current after every session** — last section first.

## Legend

- ✅ done · 🔨 in progress · ⏸ blocked (state why) · 📅 planned

## Current state (2026-10-08)

| Layer | Status |
|---|---|
| core: param/chain/config/osc/osc_io | ✅ builds, 26/26 test cases, 142/142 assertions |
| core: CMake + local build | ✅ (MSYS2 ucrt64 g++ 16.2, cmake 4.4) |
| core: CI (GH Actions, 3-OS) | 🔨 workflow committed, first run pending |
| host/ (JUCE VST2/3 backend + app) | ⏸ not started — no JUCE/MSVC on this box; must build in CI or dedicated env |
| cli/ (offline audio→audio) | ⏸ not started |
| docs (README/CONFIG/OSC/ARCHITECTURE) | ✅ |
| logo + name (Weft) | ✅ (assets/logo/) |

## Log

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
