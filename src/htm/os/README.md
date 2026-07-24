# 📁 `os/` — Minimal Filesystem Layer

> The little that remains of upstream's OS abstraction — just what
> serialization needs.

| File | Role |
|---|---|
| `Path.{hpp,cpp}` | Path manipulation helpers used by `Serializable` save/load-to-file. |
| `Directory.{hpp,cpp}` | Directory iteration/creation used alongside `Path`. |
| `ImportFilesystem.hpp` | Selects `std::filesystem` vs. experimental/boost per compiler. |

> ℹ️ Upstream also had `Timer` and `Env` here — both served only the removed
> NetworkAPI engine and went with it.
