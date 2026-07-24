#!/usr/bin/env python3
"""
Single-HTM-Core :: standalone build/install script.

Compiles the single-HTM engine (Spatial Pooler, Temporal Memory,
Connections, anomaly, SDR, encoders) on its own, with this directory as the
CMake source root. Automatic decisions -- no manual flags required:

  * SIMD level (``HTM_MARCH=auto``): the CMake probe (CPUID + XGETBV, see
    ``DetectMarch.cmake``) selects x86-64-v2/v3/v4 for the build host; the
    library is capped at v3 for numeric invariance, with an AVX-512 integer
    kernel opting in per-file on v4 hosts.
  * OS / compiler: CMake auto-detects the platform toolchain (MSVC on
    Windows, gcc/clang on Linux/macOS).

Usage:
    python htm_install.py                 # build the C++ library only
    python htm_install.py --bindings      # also build the Python modules
    python htm_install.py --march v3      # force a SIMD level (override auto)
    python htm_install.py --build-type Debug

This script is NOT used by PyHTM-core's own build (that drives the
repository-root CMakeLists). It exists so the Single-HTM-Core subtree can be
built -- and contributed upstream (e.g. a PR to htm.core) -- on its own.
"""
import argparse
import os
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
BUILD_DIR = os.path.join(HERE, "build")


def _run(cmd, **kw):
    print(">>", " ".join(cmd), flush=True)
    subprocess.run(cmd, check=True, **kw)


def _have(exe):
    return shutil.which(exe) is not None


def purge_interrupted_build_leftovers():
    """Self-heal after a build that was stopped mid-way (Ctrl+C / kill).

    This script builds INCREMENTALLY by default (no --clean), so a compiler
    or archiver killed mid-write can leave a 0-byte .o/.obj (or library)
    behind; being newer than its sources, the next build treats it as up to
    date and silently archives/links it -- the failure only surfaces later
    as 'undefined symbol'. A valid artifact of these kinds is never 0
    bytes, so deleting empty ones only forces the build system to redo the
    work it failed to finish; the build logic itself is untouched.
    """
    exts = ('.o', '.obj', '.a', '.lib', '.so', '.pyd', '.dll')
    removed = 0
    for dirpath, _dirnames, filenames in os.walk(BUILD_DIR):
        for name in filenames:
            if name.endswith(exts):
                path = os.path.join(dirpath, name)
                try:
                    if os.path.getsize(path) == 0:
                        os.remove(path)
                        removed += 1
                        print(f"Removed 0-byte leftover of an interrupted "
                              f"build: {path}")
                except OSError:
                    pass
    if removed:
        print(f"Purged {removed} interrupted-build leftover(s); "
              f"the build will regenerate them.")


def _ensure_cmake():
    """Make cmake available, installing it into the active environment if not.

    CMake ships as a pip wheel, so a Python environment can supply it without
    a system package or an administrator. That keeps this subtree buildable
    from a bare conda environment, which is how it is usually met. A cmake
    already on PATH is left alone -- a system or module-provided one is
    preferred over anything installed here.
    """
    if _have("cmake"):
        return
    print("   cmake    : not on PATH -- installing the pip wheel")
    try:
        subprocess.run([sys.executable, "-m", "pip", "install", "cmake>=3.21"],
                       check=True)
    except subprocess.CalledProcessError:
        sys.exit("ERROR: cmake is not on PATH and could not be installed.\n"
                 "       Install it yourself, then run this again:\n"
                 f"           {sys.executable} -m pip install cmake\n"
                 "       or load a system module that provides it.")
    if not _have("cmake"):
        sys.exit("ERROR: cmake was installed but is still not on PATH.\n"
                 "       The environment's scripts directory is probably not "
                 "in PATH.\n"
                 "       Reactivate the environment and run this again.")


def _ensure_pybind11():
    """Make pybind11 importable when the Python modules are being built.

    CMake locates pybind11 by asking this interpreter for its CMake package
    directory, so the wheel has to be present in the environment being built
    against. Installing it here means a bare environment can build the modules
    without a separate preparation step, matching how cmake is handled above.
    """
    try:
        import pybind11  # noqa: F401
        return
    except ImportError:
        pass
    print("   pybind11 : not installed -- installing the wheel")
    try:
        subprocess.run([sys.executable, "-m", "pip", "install",
                        "pybind11>=2.13.6"], check=True)
    except subprocess.CalledProcessError:
        sys.exit("ERROR: pybind11 is required for --bindings and could not be "
                 "installed.\n"
                 "       Install it yourself, then run this again:\n"
                 f"           {sys.executable} -m pip install pybind11\n"
                 "       Or build the C++ library only, without --bindings.")


def preflight(bindings=False):
    print("== Single-HTM-Core build: pre-flight checks ==")
    print(f"   python   : {sys.version.split()[0]} ({sys.executable})")
    print(f"   platform : {sys.platform}")
    _ensure_cmake()
    if bindings:
        _ensure_pybind11()
    # a compiler check: MSVC is located by CMake itself on Windows; on
    # POSIX we can sanity-check for a C++ compiler up front.
    if sys.platform != "win32" and not (_have("c++") or _have("g++")
                                        or _have("clang++")):
        sys.exit("ERROR: no C++ compiler found (g++/clang++). Install a "
                 "C++17 toolchain.\n"
                 "       On a cluster this usually means you are on a login "
                 "node -- request\n"
                 "       a compute node and try again.")
    print("   toolchain: OK")


def _relax_tarfile_filter():
    """Let the build extract archives that the hardened tarfile rejects.

    Python 3.12 (and 3.9.17+ / 3.11.4+ via PEP 706) refuse tar members with
    absolute links or links that point outside the destination. Some source
    archives pulled in during a build contain exactly that, and the failure
    surfaces far from its cause -- as an AbsoluteLinkError or a
    LinkOutsideDestinationError deep inside pip or a build backend.

    PYTHON_TARFILE_EXTRACTION_FILTER restores the pre-hardening behaviour for
    this process and everything it spawns, without touching the interpreter's
    own tarfile.py. It is only needed on Linux, where those archives are used,
    and an explicit setting already in the environment is left alone.
    """
    if sys.platform.startswith("linux") and \
            "PYTHON_TARFILE_EXTRACTION_FILTER" not in os.environ:
        os.environ["PYTHON_TARFILE_EXTRACTION_FILTER"] = "fully_trusted"
        print(">> PYTHON_TARFILE_EXTRACTION_FILTER=fully_trusted "
              "(Linux archive extraction)", flush=True)


def configure(args):
    cfg = [
        "cmake", "-S", HERE, "-B", BUILD_DIR,
        f"-DCMAKE_BUILD_TYPE={args.build_type}",
        f"-DBINDING_BUILD={'Python3' if args.bindings else 'CPP_Only'}",
    ]
    env = dict(os.environ)
    if args.march:
        env["HTM_MARCH"] = args.march   # override the auto-probe
    # HTM_MARCH unset -> CMake auto-probes this host.
    _run(cfg, env=env)


def build(args):
    _run(["cmake", "--build", BUILD_DIR, "--config", args.build_type,
          "-j", str(os.cpu_count() or 2)])


def main():
    ap = argparse.ArgumentParser(description="Build Single-HTM-Core standalone.")
    ap.add_argument("--bindings", action="store_true",
                    help="also build the pybind11 Python modules")
    ap.add_argument("--march", default=None,
                    help="SIMD level override (off/x86-64/v2/v3/v4/native); "
                         "default is auto-probe")
    ap.add_argument("--build-type", default="Release",
                    help="CMake build type (default: Release)")
    ap.add_argument("--clean", action="store_true",
                    help="remove the build directory first")
    args = ap.parse_args()

    _relax_tarfile_filter()

    if args.clean:
        shutil.rmtree(BUILD_DIR, ignore_errors=True)

    purge_interrupted_build_leftovers()
    preflight(bindings=args.bindings)
    configure(args)
    build(args)

    lib_hint = os.path.join(BUILD_DIR, "src")
    print("\n== build complete ==")
    print(f"   library : {lib_hint} (libhtm_core.a / htm_core.lib)")
    if args.bindings:
        print("   modules : htm.bindings.sdr / .encoders / .algorithms")
    print("   (this subtree is self-contained: it carries its own "
          "CommonCompilerConfig.cmake + DetectMarch.cmake)")


if __name__ == "__main__":
    main()