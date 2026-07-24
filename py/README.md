# 📦 `py/` — The Importable `htm` Package

> What actually lands in the wheel. `pyproject.toml` packages
> [`py/htm`](htm/README.md) as-is, and the build drops the three compiled
> extension modules into `htm/bindings/` next to the Python files.

| Path | Role |
|---|---|
| [`htm/`](htm/README.md) | The package root — see its [README](htm/README.md) for the layout. |

> ℹ️ Everything computational is C++. The Python files here are import
> surface and one pure-Python encoder. Upstream's `advanced/` (research
> code) and `optimization/` (swarming) trees were removed — PyHTM never
> imported them, and they were the only consumers of the `hexy` /
> `prettytable` dependencies, so the package now depends on **NumPy only**.