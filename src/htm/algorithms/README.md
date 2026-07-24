# ⚙️ `algorithms/` — The Learning Machinery

> Spatial Pooler, Temporal Memory, and the synapse store (Connections) —
> this is where the model actually runs, and where nearly all of
> PyHTM-core's performance work is concentrated.

## 📄 Files

| File | Role |
|---|---|
| `SpatialPooler.{hpp,cpp}` | Columns competing over input overlap. Entry point: `compute(input, learn, active)`. |
| `TemporalMemory.{hpp,cpp}` | Sequence memory over SP columns. Entry points: `activateDendrites()`, `activateCells()`, `getPredictiveCells()`, `cellsToColumns()`. |
| `Connections.{hpp,cpp}` | The synapse/segment store both algorithms share. Hosts the hottest loops in the codebase: `computeActivity`, `adaptSegment`, `growSynapses`, `createSynapse`. |
| `Anomaly.{hpp,cpp}` | `computeRawAnomalyScore(active, predicted)` — also exposed to Python at module level. |
| `AnomalyLikelihood.{hpp,cpp}` | Distribution-based anomaly likelihood (used by TM's `ANMode::LIKELIHOOD`). |

## 🚀 Performance notes (what differs from upstream)

- **Connections** — presynaptic maps are direct-indexed vectors instead of
  hash maps (CSR-style), plus `reserveBuffers()` pre-sizing for the very
  allocation-heavy model-build phase.
- **SpatialPooler** — `initialize()` reuses hoisted buffers instead of two
  fresh input-sized vectors per column. `updateDutyCycles_` consumes raw
  overlaps (no temporary SDR per step). Global inhibition keeps a reusable
  index scratch. Boost updates early-out when `boostStrength == 0`.
  `compute()` returns a movable overlaps vector.
- **TemporalMemory** — `getPredictiveCells()` deduplicates the (already
  cell-sorted) active-segment list linearly instead of via a `std::set`.

> ℹ️ **Every change above is behaviour-preserving.**
> including RNG order and float-op order.