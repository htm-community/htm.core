# 🧬 `types/` — The SDR and Core Types

> The data structure every bit of information in the system flows through.

## 📄 Files

| File | Role |
|---|---|
| `Sdr.{hpp,cpp}` | **SparseDistributedRepresentation** — lazily-cached dense/sparse/coordinate views, set algebra (`intersection`, `set_union`, `concatenate`, **`subtract`**), reshape, RNG ops. |
| `Serializable.hpp` | The cereal-based save/load machinery (binary/JSON/XML) behind `pickle` support. |
| `Types.hpp` | Fundamental typedefs (`UInt`, `Real`, `CellIdx`, `Permanence`, …) and `htm::Epsilon`. |
| `Exception.hpp` | The exception type raised by `NTA_CHECK` / `NTA_THROW`. |

## ➖ `subtract` (added for PyHTM)

```cpp
out.subtract(minuend, subtrahend).   // out = minuend AND NOT subtrahend
```

Sorted-sparse set difference in a single `std::set_difference` pass — no
dense conversion, safe to call in-place. Added because PyHTM's residual
(RTM) mode computes *active − predicted* **every step of every module** and
previously did it with Python sets (measured ×117 slower). The Python
binding releases the GIL around it.

> ℹ️ SDR views are converted lazily and cached: `getSparse()` after
> `setSparse()` is free. The first `getDense()` after a sparse write pays a
> one-time scatter. The bindings expose `sdr.sparse` as a **zero-copy**
> NumPy view into the C++ buffer.