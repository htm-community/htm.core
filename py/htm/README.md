# 🐍 `htm/` — Package Root

> The namespace PyHTM imports from. Thin by design: real work happens in the
> compiled modules under [`bindings/`](bindings/README.md).

## 🗂️ Layout

| Path | Role |
|---|---|
| `__init__.py` | Re-exports `SDR`/`Metrics` so `import htm` works standalone. |
| [`bindings/`](bindings/README.md) | Where the compiled `algorithms` / `sdr` / `encoders` modules land, + the install sanity-checker. |
| [`encoders/`](encoders/README.md) | Encoder import surface: C++ re-export shims + the pure-Python `DateEncoder`. |
| [`algorithms/`](algorithms/README.md) | Algorithm import surface (`from htm.algorithms import SpatialPooler`, …). |

## 🎯 The exact surface PyHTM uses

```python
from htm.bindings.algorithms import SpatialPooler, TemporalMemory
from htm.bindings.sdr        import SDR
from htm.encoders.rdse       import RDSE, RDSE_Parameters
from htm.encoders.date       import DateEncoder
```
