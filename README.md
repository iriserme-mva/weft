# Weft

**Weft** — a C++ audio-plugin chain host: load VST 2 / VST 3 plugins into an
ordered processing chain, drive every parameter live over **OSC**, describe the
whole setup in a small **JSON** config, and run it offline from a CLI.

> *Weft*: the longitudinal threads in weaving that the warp threads interlace
with — here, the plugins whose signals are interlaced into one chain.

![Weft banner](assets/logo/weft_banner.png)

---

## Status

| Layer | What it is | State |
|---|---|---|
| `core/` | Platform-agnostic C++17 library: param model, chain, JSON config, OSC wire + addressing, UDP transport | **building & tested** (26/26 test cases green) |
| `host/` | JUCE-based VST2/3 wrapper backend + GUI app (IO linking) | planned |
| `cli/` | Offline audio→audio CLI with fixed or time-series params | planned |

Progress is tracked in [STATUS.md](STATUS.md).

## What Weft does

1. **Chains VST2/3 plugins** in an ordered processing graph.
2. **Loads per-plugin / per-param default values from JSON** — one config file
   describes the whole rig, versionable and diff-able.
3. **Live-updates the chain** when any param changes at runtime (hot-apply:
   unchanged plugins are never reloaded).
4. **Queries all params** from each loaded plugin (normalized surface with
   name, id, range, steps, kind).
5. **Exposes dynamic per-param control over OSC** — set by *name* or by *id*,
   reset per-slot or global, re-load config by path or JSON blob, and receive
   `/change/...` pushes when a plugin changes its own params.

Architecture decision (user-directed): the **inner VST wrapper only describes
the param interface**; the **outer app owns the OSC transport**. The core
library never links a plugin SDK.

## Building the core (local)

Requires CMake ≥ 3.16 and a C++17 compiler (tested: MSYS2 ucrt64 g++ 16, cmake 4.4).

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
cd build && ctest --output-on-failure
```

Third-party headers (`nlohmann/json`, `doctest`) are vendored under
`third_party/` — no network access needed to build.

CI runs the same steps on GitHub Actions (see `.github/workflows/ci.yml`).

## Layout

```
weft/
├── core/
│   ├── include/weft/
│   │   ├── param.hpp      # Param / ParamSet model
│   │   ├── chain.hpp      # Chain + IPluginBackend
│   │   ├── config.hpp     # JSON config parse/dump/merge
│   │   ├── osc.hpp        # OSC 1.0 wire codec + /chain addressing
│   │   └── osc_io.hpp     # UDP OSC transport
│   ├── src/               # implementations
│   └── tests/             # targeted doctest suites
├── docs/                  # CONFIG.md, OSC.md, ARCHITECTURE.md
├── examples/chain.json    # example rig config
├── assets/logo/           # banner, avatar, mark
└── third_party/           # nlohmann/json.hpp, doctest.h (vendored)
```

## Documentation

- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) — layering, backend contract, data flow
- [docs/CONFIG.md](docs/CONFIG.md) — JSON schema + merge semantics
- [docs/OSC.md](docs/OSC.md) — full OSC protocol reference

## Prior art

Weft is informed by (and deliberately distinct from):

- **ReaPlugga** (ReaPlugga/ReaPlugga on GitHub) — hosts VST2 plugins in a
  ReWire/ASIO context; no param-level OSC.
- **JUCE `AudioPluginHost` / HostPluginDemo** — reference hosting; Weft builds
  the host layer on JUCE hosting and adds the chain+OSC+config layer above it.
- **getdunne/juce-plugin-wrapper** — minimal JUCE VST wrapper as a starting
  point for the backend contract.

## License

Core library: **MIT** (see `core/LICENSE`). The future `host/` layer will use
JUCE, which is GPL-3.0 *or* commercial-licensed — its license will be declared
in `host/` before first release so the overall licensing is unambiguous for
buyers.
