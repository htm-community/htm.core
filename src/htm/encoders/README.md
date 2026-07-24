# 🔡 `encoders/` — Values → SDRs

> C++ encoders that turn raw inputs into Sparse Distributed Representations.

## 📄 Files

| File | Role |
|---|---|
| `RandomDistributedScalarEncoder.{hpp,cpp}` | **RDSE** — the encoder PyHTM uses for numeric features. Hash-based bucketing (MurmurHash3 from [`external/common`](../../../external/README.md)). Nearby values share bits. |
| `ScalarEncoder.{hpp,cpp}` | Classic bounded scalar encoder. Also the building block the pure-Python `DateEncoder` composes. |
| `DateEncoder.{hpp,cpp}` | C++ date/time encoder (season, day-of-week, weekend, holiday, time-of-day…). |
| `BaseEncoder.hpp` | The tiny common interface (`size`, `dimensions`, `encode`). |

> ℹ️ **Two DateEncoders exist.** PyHTM currently imports the *pure-Python*
> one ([`py/htm/encoders/date.py`](../../../py/htm/encoders/README.md)),
> which composes ScalarEncoders. The C++ `DateEncoder` here is bound and
> available as `htm.bindings.encoders.DateEncoder`. Switching PyHTM to it is
> a candidate for the PyHTM-side optimization phase (needs parameter-mapping
> validation first).

The `RDSE.encode` binding releases the GIL — encoding different features on
different hive threads runs truly in parallel.