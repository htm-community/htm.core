# 📂 src — The C++ HTM Engine

This is the heart of PyHTM-core: the C++ implementation of HTM, compiled into
the `htm_core` library that the Python bindings call into. The layout is
unchanged from upstream htm.core, so it stays easy to follow and to track
against the original.

> You won't run anything here directly. `CMakeLists.txt` here is invoked by
> the top-level build (`htm_install.py` → CMake) to compile everything under
> `htm/` into the engine library.

---

## What's inside `htm/`

| Subfolder | Responsibility |
|-----------|----------------|
| `algorithms/` | The core learning algorithms — **Spatial Pooler**, **Temporal Memory**, **Connections** (synapse/segment storage), Anomaly / AnomalyLikelihood. This is where PyHTM-core's performance changes live (Connections buffer pre-reservation + CSR-style structures, hot-path allocation removal). |
| `encoders/` | Turn raw inputs into SDRs — scalar, date, RDSE encoders. |
| `types/` | Fundamental types — **SDR**, the serialization interfaces, and base exception/type definitions. |
| `os/` | OS-abstraction helpers — paths, directories, environment, timers. |
| `utils/` | Shared utilities — logging, random, moving average, SDR metrics, topology, sliding window. |

---

## How it builds

`src/CMakeLists.txt` compiles all of the `htm/**` sources into an object
library, then assembles the static `htm_core` library (bundling the small
vendored dependencies). The top-level build then links the Python bindings
in [`../bindings/py`](../bindings/README.md) against this library.

> The upstream `examples/` and `test/` trees that normally sit here were
> removed in this fork (not needed to build/run the engine), and the build
> files were adjusted accordingly.
