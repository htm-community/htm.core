# 🐍 `cpp_src/` — The pybind11 Sources

> Three extension modules, compiled against the static `htm_core` library.

## 📦 Modules

| Module (→ `htm/bindings/*.so`) | Sources | Exposes |
|---|---|---|
| `algorithms` | `bindings/algorithms/` | `SpatialPooler`, `TemporalMemory`, `Connections`, module-level `computeRawAnomalyScore` -- and `stepSpTm`, which runs PyHTM's whole
per-module step (SP -> TM -> anomaly -> residual) under a SINGLE release,
collapsing the 6-7 boundary crossings of the per-call path into one. |
| `sdr` | `bindings/sdr/` | `SDR` (zero-copy NumPy views, set algebra incl. `subtract`), `Metrics`. |
| `encoders` | `bindings/encoders/` | `RDSE`, `ScalarEncoder`, `DateEncoder` (+ their parameter structs). |

## 🔓 GIL strategy

Every heavy C++ entry point runs under a **released GIL**, so PyHTM's thread
hive overlaps C++ compute across models:
`SP.compute`, `TM.activateDendrites`, `TM.activateCells`,
`TM.getActiveCells`, `TM.getPredictiveCells`, `TM.getWinnerCells`,
`TM.cellsToColumns`, `RDSE.encode`, `SDR.subtract`,
`computeRawAnomalyScore` -- and `stepSpTm`, which runs PyHTM's whole
per-module step (SP -> TM -> anomaly -> residual) under a SINGLE release,
collapsing the 6-7 boundary crossings of the per-call path into one.

## 🧩 Support headers

| File | Role |
|---|---|
| `bindings/py_utils.hpp` | Shared helpers (`get_it`/`get_end` NumPy accessors). Lived under the removed `engine/` bindings upstream. Relocated here. |
| `bindings/suppress_register.hpp` | Must be included **before** `pybind11.h` (works around a register-keyword warning). |

Build wiring lives in [`CMakeLists.txt`](CMakeLists.txt). The modules are
installed into the wheel's `htm/bindings/` by scikit-build-core.