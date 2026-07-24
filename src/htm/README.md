# 🗺️ `src/htm/` — Engine Source Map

> The C++ implementation of the HTM engine, organized by component.
> Everything here compiles into the `htm_core` library that the
> [Python bindings](../../bindings/py/README.md) link against.

## 📦 Components

| Directory | What lives there | Details |
|---|---|---|
| [`algorithms/`](algorithms/README.md) | **Spatial Pooler, Temporal Memory, Connections**, anomaly scoring — the learning machinery itself. | → [README](algorithms/README.md) |
| [`encoders/`](encoders/README.md) | Raw values → SDRs: **RDSE**, Scalar, Date encoders. | → [README](encoders/README.md) |
| [`types/`](types/README.md) | The **SDR** data structure + serialization / exception plumbing. | → [README](types/README.md) |
| [`utils/`](utils/README.md) | Shared infrastructure: RNG, topology, SDR metrics, moving averages, logging. | → [README](utils/README.md) |
| [`os/`](os/README.md) | The thin filesystem layer (paths, directories). | → [README](os/README.md) |

## 🧭 Orientation

- The **hot path** of a PyHTM step is:
  `encoders/RandomDistributedScalarEncoder` → `algorithms/SpatialPooler::compute`
  → `algorithms/TemporalMemory::activateDendrites / activateCells`
  → back out through [`types/Sdr`](types/README.md).
- `Version.hpp.in` is configured by CMake into the build tree at configure
  time (git SHA + version stamp).
- Upstream htm.core also had `engine/`, `ntypes/`, and `regions/` here —
  the NetworkAPI graph framework. PyHTM drives the algorithms directly and
  never used it, so this fork removed it entirely
  (see the [root README](../../README.md#-what-was-removed)).
