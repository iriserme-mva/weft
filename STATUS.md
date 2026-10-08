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
| host: CMake (JUCE 8.0.4 sha256-pinned, opt-in WEFT_BUILD_HOST) | ✅ pushed; FORMATS VST3 wrapper fix pending CI verification |
| host: CI (3-OS matrix + smoke param assertions) | 🔨 run 37853305618 (from `1b22ca3`) RUNNING — prior failure root cause **confirmed from CI log**: smoke host missing `JUCE_PLUGINHOST_VST3=1` → format manager had zero formats → `no plugin format can load: …/WeftSmokePlugin.vst3`. Fixed in `1b22ca3`; awaiting re-run |
| cli/ (offline audio→audio) | ⏸ not started |
| docs (README/CONFIG/OSC/ARCHITECTURE) | ✅ |
| logo + name (Weft) | ✅ (assets/logo/) |

## Log

- **2026-10-09 (9)** — CI run 37845591328 (from `00769c9`): `00769c9` **worked**
  — the old `plugin file not found` error is gone, the format-manager slow path
  now runs, and all 3 OSes still build/link/package cleanly (build 47/47, smoke
  binary linked, .vst3 bundle present). Failure advanced one stage deeper, to
  the format manager itself, which printed
  `error: no plugin format can load: …/WeftSmokePlugin.vst3`.
  **Root cause confirmed from the Linux CI log (job 113545530340):** the
  `weft_smoke` compile flags carried `-DJUCE_USE_CURL=0 -DJUCE_WEB_BROWSER=0`
  but **no** `-DJUCE_PLUGINHOST_VST3=1`. The `AudioPluginFormatManager`
  registers `VST3PluginFormat` only when that per-target define is set, so the
  smoke host had zero formats and load() failed. Fix (`1b22ca3`): add
  `JUCE_PLUGINHOST_VST3=1` to `weft_smoke`'s `target_compile_definitions`,
  exactly matching JUCE's own AudioPluginHost. Link-safety verified against the
  JUCE 8.0.4 tree: `_juce_module_sources` compiles only the top-level module TU
  (`juce_audio_processors.cpp`), which `#include`s
  `format_types/juce_VST3PluginFormat.cpp` (line 216); that TU pulls in the VST3
  SDK base sources transitively under `JUCE_PLUGINHOST_VST3`. JUCE's shipping
  `AudioPluginHost` example links with only this define + the module (no
  explicit SDK base sources), proving the module system handles the SDK
  transparently — so `weft_smoke`, which links the same `juce_audio_processors`
  module, links identically. Re-run triggered as `37853305618`.
- **2026-10-08 (8)** — CI run 37810148573 (from `a7d2748`): the two packaging
  fixes from (7) **both worked** — all 3 OSes now compile, link, and package
  the complete VST3 bundle (build reached 47/47; the `moduleinfo.json`-removal
  step ran; the CI shell confirmed the bundle *directory* exists on disk).
  Failure advanced to the smoke test, which printed
  `error: plugin file not found: …/WeftSmokePlugin.vst3` — despite the bundle
  dir being present. Root cause = a bug in my smoke host, not the packaging:
  `JuceBackend::loadSlot` gated on `file.existsAsFile()`, but a VST3 plugin is
  a bundle *directory* (`WeftSmokePlugin.vst3/Contents/…`), so `existsAsFile()`
  is false and the load bailed before reaching the format manager. Fix
  (`00769c9`): accept file **or** directory via `file.exists()`, then let the
  format manager decide. Verified against JUCE 8.0.4 source that the whole
  directory-bundle chain works: `fileMightContainThisPluginType`
  (juce_VST3PluginFormat.cpp:4217) = `hasFileExtension(".vst3") && f.exists()`
  → true for a dir; `findAllTypesForFile` (4095) fast-path empty (no
  moduleinfo.json) → `findDescriptionsSlow` (4128); `createVST3Instance`
  (4153) same gate → true. CI-fix cycle 5.

- **2026-10-08 (7)** — CI run 37780116943 (from `3a1745c`): both prior C++
  errors (String ctor, private getValue) are **fixed** — all 3 OSes now compile
  and the VST3 **links**. Failures moved to the JUCE packaging stage, 2 new
  independent errors, both source-verified against JUCE 8.0.4:
  - Linux (compile, `juce_core.cpp` compiled *into* `weft_smoke_plugin`):
    `fatal error: curl/curl.h`. `JUCE_USE_CURL` defaults to 1 via a `#ifndef`
    guard (juce_core.h:151) and the runner lacks libcurl dev headers. We use
    no networking; curl *linking* is separately opt-in (`NEEDS_CURL`, default
    off) so there is nothing to unlink. Fix: `target_compile_definitions(...
    JUCE_USE_CURL=0)` on both `weft_smoke_plugin` and `weft_smoke` (module
    sources compile into each target with that target's own defs).
  - Windows + macOS (post-build): `juce_vst3_helper -create` (writes
    moduleinfo.json) crashes — Windows 0xC0000005 (MSB3073), macOS SIGSEGV
    139. No matching upstream issue found. 8.0.15 rewrote the helper
    (shared → per-plugin, inherits plugin compile defs) but also split
    `juce_audio_processors` into `_headless` — reorg risk too high; staying on
    the 8.0.4 pin. JUCE's own sanctioned workaround: `VST3_AUTO_MANIFEST` is a
    documented `juce_add_plugin()` keyword (one_value arg, property default
    TRUE, gates the helper invocation) → set FALSE. The smoke host is
    unaffected: `VST3PluginFormat::findAllTypesForFile` falls back to slow-path
    factory enumeration when moduleinfo.json is absent
    (`getLibraryPaths` → `getPluginFactory` → `findDescriptionsSlow`, verified
    at juce_VST3PluginFormat.cpp:4095-4131) — instance creation
    (`createVST3Instance` → `VST3ModuleHandle::create`) never reads it either.
  Pushed; next CI run is the source of truth. CI-fix cycle 4.

- **2026-10-08 (6)** — CI run 37761767553 (from `67bfd8d`): core green 3/3,
  host red 3/3 — but down to just **2 distinct errors**, both source-verified
  against JUCE 8.0.4:
  - JuceBackend.cpp:99 (Windows C2228 + C2660 cascade): `juce::String(path)`
    with `path` a `std::string`. JUCE 8 `String` has **no `std::string` ctor**
    (only `const char*`/`const wchar_t*`/char8_t), so the `File` ctor failed
    and `file` became an error-type — which is why MSVC then misreported
    `.existsAsFile` (C2228) and `findAllTypesForFile` as "1 argument" (C2660);
    the call site itself was correct (2-arg). Fix: `juce::String(path.c_str())`.
    (Linux/macOS never compiled JuceBackend.cpp because plugin.cpp failed first,
    so only MSVC surfaced this.)
  - plugin.cpp:33 (Linux + macOS): `AudioParameterFloat::getValue()` is
    **private** in JUCE 8; the public accessors are `get()` / `operator float()`.
    Fix: `dryMixParameter->get()`. (Note: `p->getValue()`/`p->getDefaultValue()`
    at JuceBackend.cpp:35-36 are the *public* virtuals on the
    `AudioProcessorParameter*` base pointer and are unaffected.)
  Pushed; next CI run is the source of truth. CI-fix cycle 3/3.

- **2026-10-08 (5)** — CI run 37727941884 (from `1d0849d`): host still red 3/3,
  but now at the C++ layer. Parsed the exact compiler errors from all three job
  logs and fixed each against the JUCE 8.0.4 source:
  - plugin.cpp (Linux/macOS, abstract-class + 4 errors): added the 6 missing
    pure-virtual overrides (getName, getTailLengthSeconds, acceptsMidi,
    producesMidi, getStateInformation, setStateInformation); removed the
    `getLatencySamples` override (not virtual in JUCE 8); getProgramName now
    returns `const String`; prepareToPlay reads getValue() instead of the
    private AudioParameterFloat::getDefaultValue(); processBlock uses the
    1-arg all-channels applyGain(gain) (2-arg overload doesn't exist).
  - JuceBackend.cpp (Windows, 6 errors): String::toInteger64 → new
    paramIdFromJuce() helper (getLargeIntValue, decimal VST3 ids);
    RangedAudioParameter getMin/MaxValue → getNormalisableRange().start/.end;
    File::fromString → File(String) ctor; fileMightContainThisPluginType /
    findAllTypesForFile take a String → pass getFullPathName();
    createPluginInstance(desc,…) — descs[0] is a pointer, dereferenced to a
    `const PluginDescription&`.
  - smoke.cpp: initialiseJuce_GUI is declared in juce_events — added the
    include (it was a compile error, not a link error; juce_events was already
    linked via juce_audio_processors).
  - CMake (Linux/macOS gtk/gtk.h + macOS WebKit): JUCE_WEB_BROWSER defaults to
    1 and the juce_gui_extra module .cpp files compile INTO our targets, so
    `JUCE_WEB_BROWSER=0` is set on both weft_smoke and weft_smoke_plugin
    (exactly how JUCE's own AudioPluginHost does it). No JUCE_VIDEO setting
    exists in 8.0.4 — web browser is the only gate for the GTK/WebKit blocks.
  - Note: `juce::AudioBuffer<float>` is still the type name in 8.0.4
    (AudioSampleBuffer is a newer internal rename; the Linux error itself named
    juce::AudioBuffer<float> and only complained about the applyGain call).
  Pushed; next CI run is the source of truth. CI-fix cycle 2/3.

- **2026-10-08 (4)** — CI run 37726704308: core green 3/3, host red 3/3.
  Two independent root causes (both source-verified against JUCE 8.0.4 +
  runner image readme):
  (1) Linux+macOS: `ninja: unknown target 'weft_smoke_plugin_VST3'` —
  `juce_add_plugin` parses formats via the NAMED `FORMATS` multi-arg
  (JUCEUtils.cmake cmake_parse_arguments); our positional `VST3` was
  dropped, so JUCE_FORMATS was empty and no wrapper target was ever
  created. Fix: `FORMATS VST3` in host/test_plugin/CMakeLists.txt.
  (2) Windows: runner image is windows-2025-vs2026 (VS Enterprise 2026,
  CMake 4.4.3, NO VS 2022) → generator "Visual Studio 17 2022" found no
  instance. Fix: generator "Visual Studio 18 2026" in ci.yml (CMake ≥4.2).
  Local core sanity re-green (26/26). Pushed; new run is the source of truth.
  CI-fix cycle 1/3.

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
