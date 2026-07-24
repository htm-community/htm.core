# -----------------------------------------------------------------------------
# DetectMarch.cmake -- adaptive CPU micro-architecture selection
#
# Resolves the SIMD level the build should target and exposes it as
# HTM_MARCH_FLAGS (a list, may be empty), for CommonCompilerConfig to append
# to the optimized C++ flags of BOTH cores and BOTH build stages.
#
# Selection order:
#   1. If the incoming C/C++ flags already carry an explicit -march= / -mcpu=
#      (GCC/Clang) or /arch: (MSVC) -- e.g. injected by PyHTM's
#      build_pyhtm_core.py wrapper via CXXFLAGS -- those win; this module
#      adds NOTHING.
#   2. HTM_MARCH cache variable / environment variable, when set to an
#      explicit level: off | x86-64 | x86-64-v2 | x86-64-v3 | x86-64-v4 |
#      native.
#   3. HTM_MARCH=auto (the default): compile-and-run a tiny CPUID probe on
#      the build host (checks CPU features AND OS state-saving via XGETBV)
#      and pick the highest x86-64 level the host fully supports.
#
# Fallbacks: cross-compilation, non-x86 hosts, probe failure, or a compiler
# too old for -march=x86-64-vN all degrade gracefully to baseline (no extra
# flags) with a STATUS message -- the build never breaks because of this
# module.
#
# Usage (already wired in CommonCompilerConfig.cmake):
#   include(DetectMarch)
#   htm_detect_march()            # -> HTM_MARCH_FLAGS, HTM_MARCH_RESOLVED
# -----------------------------------------------------------------------------

include_guard(GLOBAL)
include(CheckCXXCompilerFlag)

set(HTM_MARCH "auto" CACHE STRING
    "Target CPU level: auto|off|x86-64|x86-64-v2|x86-64-v3|x86-64-v4|native")
set_property(CACHE HTM_MARCH PROPERTY STRINGS
             auto off x86-64 x86-64-v2 x86-64-v3 x86-64-v4 native)

# The CPUID probe: prints the highest fully-supported x86-64 level (0/2/3/4).
# Checks OSXSAVE + XGETBV so AVX/AVX-512 are only reported when the OS
# actually saves the register state (a CPU-flag-only check can select code
# the OS cannot run).
set(_HTM_MARCH_PROBE_SRC "
#include <cstdio>
#if defined(_MSC_VER)
  #include <intrin.h>
  static void cpuidex(int r[4], int leaf, int sub) { __cpuidex(r, leaf, sub); }
  static unsigned long long xgetbv0() { return _xgetbv(0); }
#elif defined(__x86_64__) || defined(__i386__)
  #include <cpuid.h>
  #include <immintrin.h>
  static void cpuidex(int r[4], int leaf, int sub) {
    unsigned a, b, c, d;
    __cpuid_count(leaf, sub, a, b, c, d);
    r[0] = (int)a; r[1] = (int)b; r[2] = (int)c; r[3] = (int)d;
  }
  static unsigned long long xgetbv0() {
    unsigned eax, edx;
    __asm__ volatile(\"xgetbv\" : \"=a\"(eax), \"=d\"(edx) : \"c\"(0));
    return ((unsigned long long)edx << 32) | eax;
  }
#else
  int main() { std::printf(\"0\"); return 0; }
  #define HTM_NO_X86
#endif
#ifndef HTM_NO_X86
int main() {
  int r[4];
  cpuidex(r, 1, 0);
  const bool sse42   = (r[2] >> 20) & 1;
  const bool popcnt  = (r[2] >> 23) & 1;
  const bool osxsave = (r[2] >> 27) & 1;
  const bool avx_cpu = (r[2] >> 28) & 1;
  const bool fma     = (r[2] >> 12) & 1;
  cpuidex(r, 7, 0);
  const bool avx2      = (r[1] >> 5)  & 1;
  const bool bmi2      = (r[1] >> 8)  & 1;
  const bool avx512f   = (r[1] >> 16) & 1;
  const bool avx512dq  = (r[1] >> 17) & 1;
  const bool avx512cd  = (r[1] >> 28) & 1;
  const bool avx512bw  = (r[1] >> 30) & 1;
  const bool avx512vl  = (r[1] >> 31) & 1;
  unsigned long long xcr0 = osxsave ? xgetbv0() : 0;
  const bool os_avx    = (xcr0 & 0x6)  == 0x6;    // XMM+YMM state saved
  const bool os_avx512 = (xcr0 & 0xE6) == 0xE6;   // + opmask/ZMM state
  int level = 0;
  if (sse42 && popcnt) level = 2;
  if (level == 2 && avx_cpu && avx2 && fma && bmi2 && os_avx) level = 3;
  if (level == 3 && avx512f && avx512dq && avx512cd && avx512bw && avx512vl
      && os_avx512) level = 4;
  std::printf(\"%d\", level);
  return 0;
}
#endif
")

# Map a level name to per-compiler flags, verifying compiler support and
# degrading (with a message) when the toolchain is too old for -march=vN.
function(_htm_march_flags_for level outvar)
  set(flags "")
  if(level STREQUAL "off" OR level STREQUAL "x86-64")
    set(${outvar} "" PARENT_SCOPE)
    return()
  endif()
  if(MSVC)
    if(level STREQUAL "native")
      # MSVC has no /arch:native; resolve via the probe result if available.
      set(level "${HTM_MARCH_DETECTED_LEVELNAME}")
    endif()
    if(level STREQUAL "x86-64-v4")
      set(flags /arch:AVX512)
    elseif(level STREQUAL "x86-64-v3")
      set(flags /arch:AVX2)
    endif()   # v2: MSVC x64 baseline already assumes SSE2; no flag below AVX
  else()
    if(level STREQUAL "native")
      check_cxx_compiler_flag("-march=native" _htm_has_native)
      if(_htm_has_native)
        set(flags -march=native)
      endif()
    else()
      check_cxx_compiler_flag("-march=${level}" _htm_has_${level})
      if(_htm_has_${level})
        set(flags -march=${level})
      else()
        # Toolchain predates the x86-64-vN aliases (GCC<11 / Clang<12):
        # use the equivalent explicit feature set.
        if(level STREQUAL "x86-64-v4")
          set(flags -mavx512f -mavx512dq -mavx512cd -mavx512bw -mavx512vl
                    -mavx2 -mfma -mbmi2)
        elseif(level STREQUAL "x86-64-v3")
          set(flags -mavx2 -mfma -mbmi2 -mf16c -mmovbe)
        elseif(level STREQUAL "x86-64-v2")
          set(flags -msse4.2 -mpopcnt)
        endif()
        message(STATUS "HTM_MARCH: compiler lacks -march=${level}; using "
                       "explicit feature flags: ${flags}")
      endif()
    endif()
  endif()
  set(${outvar} "${flags}" PARENT_SCOPE)
endfunction()

# Main entry: resolves HTM_MARCH_RESOLVED + HTM_MARCH_FLAGS in parent scope.
macro(htm_detect_march)
  set(HTM_MARCH_FLAGS "")
  set(HTM_MARCH_RESOLVED "off")
  # The htm_core LIBRARY is capped at x86-64-v3: v4 codegen was measured to
  # perturb its floating-point results (ULP-class, enough to flip an
  # encoder boundary), while v3 is bit-identical to baseline. Integer
  # AVX-512 kernels inside the library opt in per-file (see the pyramid
  # root CMake and Connections.cpp). The pyramid runtime itself runs at
  # the full detected level (its double math is contraction-pinned and
  # verified).
  set(_htm_lib_cap 3)

  # Environment variable keeps parity with the existing HTM_MARCH env
  # convention used by PyHTM's build wrapper.
  if(DEFINED ENV{HTM_MARCH} AND HTM_MARCH STREQUAL "auto")
    set(HTM_MARCH "$ENV{HTM_MARCH}")
  endif()

  # 1. Explicit user flags always win -- add nothing on top of them.
  string(FIND "${CMAKE_CXX_FLAGS} $ENV{CXXFLAGS}" "-march=" _htm_user_march)
  string(FIND "${CMAKE_CXX_FLAGS} $ENV{CXXFLAGS} $ENV{CL}" "/arch:" _htm_user_arch)
  string(FIND "${CMAKE_CXX_FLAGS} $ENV{CXXFLAGS}" "-mcpu=" _htm_user_mcpu)
  if(NOT _htm_user_march EQUAL -1 OR NOT _htm_user_arch EQUAL -1
     OR NOT _htm_user_mcpu EQUAL -1)
    set(HTM_MARCH_RESOLVED "user-flags")
    message(STATUS "HTM_MARCH: explicit -march//arch: found in user flags; "
                   "adaptive selection disabled")
  elseif(HTM_MARCH STREQUAL "off")
    message(STATUS "HTM_MARCH: off (baseline codegen)")
  elseif(NOT HTM_MARCH STREQUAL "auto")
    set(HTM_MARCH_RESOLVED "${HTM_MARCH}")
    _htm_march_flags_for("${HTM_MARCH}" HTM_MARCH_FLAGS)
    message(STATUS "HTM_MARCH: ${HTM_MARCH} (requested) -> ${HTM_MARCH_FLAGS}")
  else()
    # 2. auto -- probe the build host once and cache the answer.
    if(NOT DEFINED HTM_MARCH_DETECTED)
      if(CMAKE_CROSSCOMPILING)
        set(HTM_MARCH_DETECTED 0 CACHE INTERNAL "detected x86-64 level")
        message(STATUS "HTM_MARCH: cross-compiling; probe skipped (baseline)")
      else()
        file(WRITE "${CMAKE_BINARY_DIR}/htm_march_probe.cpp"
             "${_HTM_MARCH_PROBE_SRC}")
        try_run(_htm_probe_run _htm_probe_compile
                "${CMAKE_BINARY_DIR}/htm_march_probe_bin"
                "${CMAKE_BINARY_DIR}/htm_march_probe.cpp"
                RUN_OUTPUT_VARIABLE _htm_probe_out)
        if(_htm_probe_compile AND _htm_probe_run EQUAL 0
           AND _htm_probe_out MATCHES "^[0-9]$")
          set(HTM_MARCH_DETECTED "${_htm_probe_out}"
              CACHE INTERNAL "detected x86-64 level")
        else()
          set(HTM_MARCH_DETECTED 0 CACHE INTERNAL "detected x86-64 level")
          message(STATUS "HTM_MARCH: probe failed; falling back to baseline")
        endif()
      endif()
    endif()
    if(HTM_MARCH_DETECTED GREATER 1)
      set(_htm_level ${HTM_MARCH_DETECTED})
      # This subtree always builds the htm_core library, whether or not the
      # Python modules are built alongside it, so the cap always applies.
      # Measured on this workload, x86-64-v4 is also SLOWER than v3: the
      # AVX-512 downclock costs more than the wider vectors return, since the
      # hot paths are sparse memory access rather than dense float math. The
      # one loop that does benefit, the integer histogram in
      # Connections::computeActivity, opts into AVX-512 per file from the
      # detected level, so nothing is given up by holding the rest at v3.
      if(_htm_level GREATER _htm_lib_cap)
        message(STATUS "HTM_MARCH: library capped at x86-64-v${_htm_lib_cap} "
                       "(detected v${_htm_level}. numeric invariance -- "
                       "integer AVX-512 kernels opt in per-file)")
        set(_htm_level ${_htm_lib_cap})
      endif()
      set(HTM_MARCH_DETECTED_LEVELNAME "x86-64-v${_htm_level}")
      set(HTM_MARCH_RESOLVED "${HTM_MARCH_DETECTED_LEVELNAME}")
      _htm_march_flags_for("${HTM_MARCH_DETECTED_LEVELNAME}" HTM_MARCH_FLAGS)
    else()
      set(HTM_MARCH_RESOLVED "x86-64")
    endif()
    message(STATUS "HTM_MARCH: auto-detected level ${HTM_MARCH_DETECTED} "
                   "-> ${HTM_MARCH_RESOLVED} ${HTM_MARCH_FLAGS}")
  endif()
endmacro()