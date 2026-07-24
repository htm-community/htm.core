# ⚡ `bindings/` — Compiled Extension Modules

> At build time, scikit-build-core installs `algorithms*.so`, `sdr*.so`
> and `encoders*.so` into this directory (sources:
> [`bindings/py/cpp_src`](../../../bindings/py/cpp_src/README.md)).

| File | Role |
|---|---|
| `__init__.py` | Package marker (Python 3 imports the `.so` files directly. The old Python-2 loader shim was removed). |
| `check.py` | Post-install sanity check — imports each extension module and runs a tiny SP loop. Handy after building on a new machine: `python -c "from htm.bindings.check import checkMain; checkMain()"`. |