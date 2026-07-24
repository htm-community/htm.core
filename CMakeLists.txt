# -----------------------------------------------------------------------------
# Single-HTM-Core -- STANDALONE build entry point.
#
# This file makes the Single-HTM-Core engine buildable on its own, with this
# directory as the CMake source root:
#
#     cmake -S Single-HTM-Core -B build -DCMAKE_BUILD_TYPE=Release
#     cmake --build build
#
# It is intended for using / contributing the single-HTM engine independently
# (e.g. a pull request back to upstream htm.core). It is NOT used by
# PyHTM-core's own build: htm_install.py drives the repository-root
# CMakeLists.txt, which compiles Single-HTM-Core/src and the bindings
# directly and never configures this file. The two paths are independent.
#
# Copies of CommonCompilerConfig.cmake and DetectMarch.cmake live in this
# directory so the subtree carries everything it needs to compile alone.
# -----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.21)
project(htm_core CXX C)

# BINDING_BUILD is a STRING, not a boolean: option() would coerce it to
# ON/OFF and the "CPP_Only" default would never be seen, which silently
# changes what DetectMarch does (the library cap only applies to a CPP_Only
# build) and leaves the value unusable in a STREQUAL comparison.
set(BINDING_BUILD "CPP_Only" CACHE STRING
    "Python3 to build pybind11 modules, CPP_Only for the C++ library only")
set_property(CACHE BINDING_BUILD PROPERTY STRINGS CPP_Only Python3)

# This directory is both the module path (for CommonCompilerConfig /
# DetectMarch / external modules) and the source root the sub-CMakeLists
# anchor on via PROJECT_SOURCE_DIR.
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
get_filename_component(REPOSITORY_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
set_property(GLOBAL PROPERTY REPOSITORY_DIR "${REPOSITORY_DIR}")
cmake_policy(SET CMP0135 NEW)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# Version: prefer an explicit -DPROJECT_VERSION1=..., else the VERSION file,
# else a placeholder. (Upstream htm.core supplies this via its own flow.)
if(DEFINED PROJECT_VERSION1)
  set(VERSION ${PROJECT_VERSION1})
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")
  file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" VERSION)
  string(STRIP "${VERSION}" VERSION)
else()
  set(VERSION "0.0.0")
endif()
message(STATUS "Single-HTM-Core standalone build -- VERSION ${VERSION}")

# Shared toolchain flags + adaptive SIMD (same modules the repo root uses).
include(CommonCompilerConfig)

set(core_library htm_core)
if(MSVC)
  set(src_lib_shared)
else()
  set(src_lib_shared htm_core_shared)
endif()

# External dependencies (cereal, MurmurHash3) -- anchored on REPOSITORY_DIR.
# (The repo-root build uses external/bootstrap.cmake, which hardcodes the
# nested layout; standalone we include the same two modules directly so the
# subtree needs no path edits.)
include(${CMAKE_CURRENT_SOURCE_DIR}/external/common.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/external/cereal.cmake)
set(EXTERNAL_INCLUDES "")
list(APPEND EXTERNAL_INCLUDES "${cereal_INCLUDE_DIR}")
list(APPEND EXTERNAL_INCLUDES "${common_INCLUDE_DIR}")

# The C++ library itself.
add_subdirectory(src)

# Optionally also build the pybind11 modules against the just-built library.
#
# The library and its headers are taken from where THIS build actually puts
# them, not from an install prefix. A parent project may stage an install
# first and point these at it; on their own, `cmake -S . -B build
# -DBINDING_BUILD=Python3 && cmake --build build` has to work in one shot,
# with no install step, which is what the paths below give.
if(BINDING_BUILD STREQUAL "Python3")
  set(HTM_CORE_LIB_DIR "${CMAKE_BINARY_DIR}/src"
      CACHE PATH "Path to the built htm_core library")

  # Headers of the library itself, the generated ones (Version.hpp), and the
  # external ones cereal.cmake / common.cmake resolved above.
  list(APPEND CORE_LIB_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/src")
  list(APPEND CORE_LIB_INCLUDES "${CMAKE_BINARY_DIR}/src")
  list(APPEND CORE_LIB_INCLUDES ${EXTERNAL_INCLUDES})

  # An install prefix, if one was staged, still takes part.
  list(APPEND CORE_LIB_INCLUDES "${CMAKE_BINARY_DIR}/Release/include")

  add_subdirectory(bindings/py/cpp_src)
endif()