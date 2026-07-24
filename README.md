# Single-HTM-Core

A C++ implementation of the core **Hierarchical Temporal Memory (HTM)**
building blocks, with Python bindings. This is a self-contained engine: the
Spatial Pooler, Temporal Memory, `Connections`, anomaly scoring, the SDR
type, and the scalar / RDSE / date encoders.

It can be built and used entirely on its own (this README), and it is the
foundation the [Pyramid-Model-Core](../Pyramid-Model-Core/README.md) runtime
is layered on top of within [PyHTM-core](../README.md).

---

## What's inside

```
Single-HTM-Core/
├── src/htm/
│   ├── algorithms/     # SpatialPooler, TemporalMemory, Connections, Anomaly
│   ├── encoders/       # ScalarEncoder, RDSE, DateEncoder, base
│   ├── types/          # SDR (Sparse Distributed Representation), Sdr metrics
│   ├── utils/          # Random, Topology, MovingAverage, ...
│   ├── os/             # minimal Directory / Path helpers
│   └── CMakeLists.txt  # the library target
├── bindings/py/cpp_src/ # pybind11 modules: algorithms / sdr / encoders
├── py/htm/             # the importable `htm` Python package (thin wrappers)
├── external/           # cereal fetch + bundled MurmurHash3
├── CMakeLists.txt          # standalone build entry (this directory as root)
├── CommonCompilerConfig.cmake
├── DetectMarch.cmake       # CPUID/XGETBV SIMD-level probe
├── htm_install.py          # standalone build/install script
└── runner_example.py       # worked example: one HTM over a CSV
```

The subtree is **self-contained**: it carries its own
`CommonCompilerConfig.cmake` and `DetectMarch.cmake`, so it configures with
this directory as the CMake source root without any parent-repository files.

---

## Building

The quickest path (auto-detects OS, compiler, and SIMD level):

```bash
python htm_install.py                # build the C++ library only
python htm_install.py --bindings     # also build the Python modules
```

Options: `--march <off|x86-64|x86-64-v2|x86-64-v3|x86-64-v4|native>` to force a
SIMD level (default is the auto-probe), `--build-type Debug`, `--clean`.

Or drive CMake directly:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBINDING_BUILD=CPP_Only
cmake --build build --config Release
# -> build/src/libhtm_core.a  (htm_core.lib on Windows)
```

Set `-DBINDING_BUILD=Python3` to also build the pybind11 extension modules.

### Requirements

* CMake ≥ 3.21 and a C++17 compiler (MSVC on Windows. Gcc/clang on
  Linux/macOS).
* For the Python bindings: CPython ≥ 3.9 and pybind11 in the environment
  being built against (`pip install pybind11`). CMake locates it by asking
  that interpreter for its pybind11 CMake directory, so no
  `-Dpybind11_DIR=...` is required. Passing one takes precedence.
* On Linux the build sets `PYTHON_TARFILE_EXTRACTION_FILTER=fully_trusted`
  for its own process, so that source archives containing absolute or
  escaping tar links extract under Python 3.12 and 3.9.17+ / 3.11.4+. An
  explicit value already present in the environment is left as it is.

### SIMD level

`HTM_MARCH=auto` (the default) probes the build host with CPUID + XGETBV and
selects `x86-64-v2/v3/v4`.

The htm_core library is always built at `x86-64-v3`, whichever level the
probe finds. Level 4 changes its floating-point results by roughly one unit
in the last place, which is enough to move a value across an encoder bucket
boundary and send two machines down different learning paths from the same
input. It is also slower here: the AVX-512 downclock costs more than the
wider vectors return, because the hot paths are sparse memory access rather
than dense float arithmetic.

The one loop that does benefit is the integer histogram in
`Connections::computeActivity`, and it opts into an AVX-512 kernel per file
from the detected level rather than the capped one. That work is exact
integer arithmetic, so it is order independent. Level 4 hosts get it, level 2
and 3 hosts never see an AVX-512 instruction.

Floating-point contraction is pinned off under march levels, so results are
identical across levels.

---

## Using it

**C++** — link the static `htm_core` library and include `src/htm`:

```cpp
#include <htm/algorithms/SpatialPooler.hpp>
#include <htm/algorithms/TemporalMemory.hpp>
#include <htm/types/Sdr.hpp>
// build an SDR, run SP -> TM, read anomaly. See the headers in src/htm/.
```

**Python** (after `--bindings`) — the modules install under `htm.bindings`:

```python
from htm.bindings.sdr import SDR
from htm.bindings.algorithms import SpatialPooler, TemporalMemory
from htm.bindings.encoders import RDSE, RDSE_Parameters

enc = RDSE(RDSE_Parameters())   # configure resolution/size/sparsity
sp  = SpatialPooler(...)        # input -> active columns
tm  = TemporalMemory(...)       # columns -> predictive cells, anomaly
```

---

## Trying it out

`runner_example.py` runs one HTM over a CSV, a row at a time: encoder →
Spatial Pooler → Temporal Memory → anomaly score.

```bash
python htm_install.py --bindings     # build the modules
# edit the EDIT THIS block at the top of runner_example.py, then:
python runner_example.py
```

Everything meant to be changed is in that block: the CSV path, which columns to
feed the model, and the encoder / SP / TM parameters. The script imports the
installed `htm` package if there is one and otherwise picks the modules
straight out of `build/`, so it runs immediately after a build with nothing
installed.

It is a *single* HTM. Several columns are encoded and concatenated into one
input SDR, not given models of their own. Multi-model hierarchies are not part
of this subtree.

---

## License & attribution

Single-HTM-Core derives from
[htm.core](https://github.com/htm-community/htm.core) (HTM Community Edition
of NuPIC. © 2013–2024 Numenta, Inc. and the htm.core community) and is
licensed under the **GNU Affero General Public License v3 (AGPLv3)**.
Original copyright notices and source-file headers are preserved. As required
by the AGPL, derivative works remain under AGPLv3.