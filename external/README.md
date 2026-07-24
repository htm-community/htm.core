# 🌐 `external/` — Third-Party Dependencies

> Down to exactly two. Each `.cmake` file fetches/wires one dependency at
> configure time.

| Dependency | File | Used by | Fetched from |
|---|---|---|---|
| **cereal** 1.3.2 | `cereal.cmake` | `Serializable.hpp` — all save/load + pickling. Header-only. | GitHub (release tarball) |
| **MurmurHash3** | `common.cmake` + `common/` | The RDSE encoder's hash bucketing. | **bundled** (compiled into the lib) |

`bootstrap.cmake` is the entry point the top-level build includes. It also
assembles `EXTERNAL_INCLUDES`.

## 📴 Air-gapped / offline builds

Pre-place a dependency under `build/Thirdparty/<name>` and the fetch step is
skipped (cereal is the only download).

> ℹ️ Upstream fetched seven more here — eigen, mnist, gtest, sqlite3,
> cpp-httplib, digestpp, libyaml. None are referenced by any remaining code
> (eigen/mnist appeared only in comments), so they were removed. Bonus:
> eigen was the only gitlab.com fetch — every remaining download is
> GitHub/PyPI, which matters on restricted networks (BGU cluster included).