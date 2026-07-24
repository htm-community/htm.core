# 📂 bindings/py — Python Bindings (pybind11)

This is the layer that exposes the C++ engine in [`../../src`](../../src/README.md)
to Python. It compiles into the extension modules that the importable `htm`
package loads, so Python code can drive the Spatial Pooler, Temporal Memory,
encoders, SDR, and NetworkAPI directly.

> This is also where one of PyHTM-core's key changes lives: the **GIL is
> released** around the heavy C++ compute calls, which is what allows PyHTM
> to run many HTM machines on real threads.

---

## Layout

| Path | What it is |
|------|------------|
| `cpp_src/` | The pybind11 binding sources — one module per area, each wrapping the matching C++ component. |
| `cpp_src/bindings/algorithms/` | Bindings for Spatial Pooler, Temporal Memory, Connections, plus the module-level `computeRawAnomalyScore`. **The GIL-release edits are here.** |
| `cpp_src/bindings/encoders/` | Bindings for the encoders (scalar, date, RDSE) — `RDSE.encode` is GIL-released. |
| `cpp_src/bindings/sdr/` | Bindings for SDR and SDR metrics. |

---

## How it builds

The top-level build (via `htm_install.py` → CMake → scikit-build-core)
compiles each module under `cpp_src/` against the static `htm_core` library
produced from [`../../src`](../../src/README.md), then packages the resulting
extension modules into the `htm` wheel that gets installed.

> The upstream binding `tests/` and `plugin/unittest/` trees were removed in
> this fork — they aren't needed to build or run the bindings.
