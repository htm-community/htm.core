# ⚙️ `algorithms/` — Python Import Surface for Algorithms

> A single `__init__.py` that re-exports the compiled module, so both
> spellings work:

```python
from htm.algorithms          import SpatialPooler, TemporalMemory   # via this shim
from htm.bindings.algorithms import SpatialPooler, TemporalMemory   # direct (what PyHTM uses)
```

Upstream's pure-Python `anomaly.py` / `anomaly_likelihood.py` lived here.
PyHTM carries its own copies, and the core exposes
`computeRawAnomalyScore` + `AnomalyLikelihood` natively — so they were
removed.