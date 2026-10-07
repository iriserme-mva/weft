# Weft config format (JSON)

A Weft config describes the whole rig: audio rates and an ordered list of
plugin slots with per-param normalized values.

```json
{
  "sample_rate": 48000,
  "block_size": 1024,
  "chain": [
    {
      "id": "comp",
      "plugin": "/opt/vst/Comp.vst3",
      "plugin_id": "com.example.comp",
      "enabled": true,
      "params": {
        "Threshold": 0.6,
        "Ratio": 0.3,
        "Enable": 1.0
      }
    },
    {
      "id": "dly",
      "plugin": "/opt/vst/Dly.vst3",
      "enabled": false
    }
  ]
}
```

## Top level

| Key | Type | Required | Default | Notes |
|---|---|---|---|---|
| `sample_rate` | int | no | 44100 | Hz |
| `block_size` | int | no | 512 | frames per block |
| `chain` | array | **yes** | — | must be an array (may be empty) |

## Slot entries (each element of `chain`)

| Key | Type | Required | Default | Notes |
|---|---|---|---|---|
| `id` | string | **yes** | — | unique within the chain; used as the slot token in OSC addresses |
| `plugin` | string | **yes** | — | path to `.vst3` / `.vst` |
| `plugin_id` | string | no | — | VST3 class id / VST2 factory id for disambiguation |
| `enabled` | bool | no | `true` | disabled slots keep their OSC address but load no plugin |
| `params` | object | no | — | `name` → normalized value in `[0, 1]` |

### Validation rules (parse fails with a message on violation)

- root must be a JSON object; `chain` must be an array
- every slot needs `id` and `plugin`
- `id` values must be unique
- param values must be numbers (clamped to `[0,1]` when applied)
- param names that don't exist on the plugin are kept in the config (they may
  appear after a hot-swap) but are not applied

## Normalized values

All params are stored and transmitted **normalized to `[0,1]`**, independent of
the plugin's native units. Conversion to/from plugin units (and to/from
string display values) happens inside the backend layer, so configs stay
stable across plugin updates.

## Merge semantics

`config::merge(base, override)` (used for layered configs, e.g. rig defaults +
per-session overrides):

- scalars: override wins when present/positive
- slots: matched by `id`; override wins, but **param maps are unioned** with
  override winning per-param — base-only params survive
- base-only slots are kept; override-only slots are appended

## Example

See [`examples/chain.json`](../examples/chain.json).
