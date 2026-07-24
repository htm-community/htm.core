# 🧰 `utils/` — Shared Infrastructure

> Everything the algorithms lean on that isn't itself an algorithm.

## 📄 Files

| File | Role |
|---|---|
| `Random.{hpp,cpp}` | The deterministic, platform-independent RNG. **Seed + call-order = reproducibility**, which is why the optimization pass never reorders RNG calls. |
| `Topology.{hpp,cpp}` | `Neighborhood` / `WrappingNeighborhood` iteration used by the SP's potential-pool mapping and local inhibition. |
| `SdrMetrics.{hpp,cpp}` | Sparsity / activation-frequency / overlap metrics (exposed as `htm.bindings.sdr.Metrics`). |
| `MovingAverage.{hpp,cpp}`, `SlidingWindow.hpp` | Rolling statistics used by AnomalyLikelihood. |
| `Log.{hpp,cpp}` | `NTA_*` logging macros. `Log.cpp` holds the `NTA_LOG_LEVEL` definition — upstream buried it in `engine/Network.cpp`. With the engine gone it moved to its natural home here. |
| `VectorHelpers.hpp` | Small vector conveniences (e.g. `sparseToBinary`). |

> ℹ️ `NTA_ASSERT` compiles out in Release builds (`NDEBUG`). `NTA_CHECK`
> stays. This fork builds Release on all platforms — on Linux upstream used
> to leave assertions in, which dominated runtime.