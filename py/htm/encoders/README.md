# 🔡 `encoders/` — Python Import Surface for Encoders

> Mostly 4-line re-export shims of the C++ classes — plus the one remaining
> pure-Python encoder.

| File | Nature | Role |
|---|---|---|
| `rdse.py` | shim | `RDSE`, `RDSE_Parameters` → re-export of the C++ [`RandomDistributedScalarEncoder`](../../../src/htm/encoders/README.md). |
| `scalar_encoder.py` | shim | `ScalarEncoder`, `ScalarEncoderParameters` → re-export of the C++ class. |
| `date.py` | **pure Python** | The `DateEncoder` PyHTM imports — composes C++ ScalarEncoders per attribute (season, weekend, time-of-day, …). |

> ℹ️ **`date.py` is the only Python file in the wheel that computes.** A
> native C++ `DateEncoder` also exists and is already bound
> (`htm.bindings.encoders.DateEncoder`). Moving PyHTM onto it is a
> PyHTM-phase candidate — the parameter semantics differ slightly, so it
> needs an equivalence check first.