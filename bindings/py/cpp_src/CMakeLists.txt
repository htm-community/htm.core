# -----------------------------------------------------------------------------
# HTM Community Edition of NuPIC
# Copyright (C) 2013-2015, Numenta, Inc.
#   Migrated to scikit-build-core:  David Keeney, dkeeney@gmail.com, Dec 2024
#   Trimmed for PyHTM-core: only the three binding modules PyHTM actually
#   imports are built -- algorithms.so, sdr.so, encoders.so. The
#   engine_internal.so / math.so modules and the PyBindRegion plugin wrapped
#   the removed Network-engine and are gone with it.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Affero Public License for more details.
#
# You should have received a copy of the GNU Affero Public License
# along with this program.  If not, see http://www.gnu.org/licenses.
# -----------------------------------------------------------------------------
#
# This builds the Python language interface.  This consists of the following libs
#   algorithms.so
#   encoders.so
#   sdr.so
#
# For PyBind11 info See: https://pybind11.readthedocs.io/en/stable/compiling.html
#######################################################################

cmake_minimum_required(VERSION 3.21)
project(htm_core CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_VERBOSE_MAKEFILE OFF) # toggle for cmake debug
message(STATUS "Configuring Python interface")
message(STATUS "   installing in ${CMAKE_INSTALL_PREFIX}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR= ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CORE_LIB_INCLUDES: ${CORE_LIB_INCLUDES}")

# Find Python
find_package(Python COMPONENTS Interpreter Development REQUIRED)

# pybind11.
#
# Try the normal lookup first: a parent project, a caller passing
# -Dpybind11_DIR=..., or a build backend such as scikit-build-core will all
# have set things up already, and that path must keep working untouched.
#
# If nothing has, ask the Python interpreter we just found where its own
# pybind11 CMake package lives. pybind11 ships that path via
# pybind11.get_cmake_dir(), so a plain `pip install pybind11` in the active
# environment is enough and this subtree needs no parent project and no
# caller-supplied flag to configure on its own.
find_package(pybind11 CONFIG QUIET)

if(NOT pybind11_FOUND)
    execute_process(
        COMMAND "${Python_EXECUTABLE}" -c
                "import pybind11, sys; sys.stdout.write(pybind11.get_cmake_dir())"
        OUTPUT_VARIABLE _htm_pybind11_dir
        RESULT_VARIABLE _htm_pybind11_rc
        ERROR_VARIABLE  _htm_pybind11_err
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(_htm_pybind11_rc EQUAL 0 AND IS_DIRECTORY "${_htm_pybind11_dir}")
        message(STATUS "pybind11 located via ${Python_EXECUTABLE}: ${_htm_pybind11_dir}")
        set(pybind11_DIR "${_htm_pybind11_dir}" CACHE PATH
            "pybind11 CMake package directory" FORCE)
        find_package(pybind11 CONFIG REQUIRED)
    else()
        message(FATAL_ERROR
            "pybind11 not found, and ${Python_EXECUTABLE} cannot import it.\n"
            "  The Python bindings need pybind11 in the environment that is "
            "being built against.\n"
            "  Install it there:\n"
            "      ${Python_EXECUTABLE} -m pip install pybind11\n"
            "  or point CMake at an existing copy:\n"
            "      -Dpybind11_DIR=<path to share/cmake/pybind11>\n"
            "  To build the C++ library only, configure with "
            "-DBINDING_BUILD=CPP_Only.")
    endif()
endif()

# How the modules refer to the library.
#
# When it is built in this same configure, link by TARGET NAME. CMake then
# resolves the file itself, which matters for multi-config generators such as
# Visual Studio that place outputs under a per-configuration subdirectory
# (build/src/Release/htm_core.lib, not build/src/htm_core.lib), and it also
# orders the build so the library is finished first.
#
# When a parent project stages the library from an earlier build stage its
# target does not exist here, so fall back to the explicit path.
if(TARGET htm_core)
    set(htm_core_lib_path htm_core)
    message(STATUS "htm_core_lib_path = htm_core (target in this build)")
else()
    set(htm_core_lib_path  "${HTM_CORE_LIB_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}htm_core${CMAKE_STATIC_LIBRARY_SUFFIX}")
    message(STATUS "htm_core_lib_path = ${htm_core_lib_path}")
endif()
#
# Python support
#
set(src_py_algorithms_files
    bindings/algorithms/algorithm_module.cpp
    bindings/algorithms/py_Connections.cpp
    bindings/algorithms/py_TemporalMemory.cpp
    bindings/algorithms/py_SpatialPooler.cpp
    )

set(src_py_sdr_files
    bindings/sdr/sdr_module.cpp
    bindings/sdr/py_SDR.cpp
    bindings/sdr/py_SDR_Metrics.cpp
    )

set(src_py_encoders_files
    bindings/encoders/encoders_module.cpp
    bindings/encoders/py_ScalarEncoder.cpp
    bindings/encoders/py_RDSE.cpp
    bindings/encoders/py_DateEncoder.cpp
    )

#set up file tabs in Visual Studio
source_group("algorithms" FILES ${src_py_algorithms_files})
source_group("sdr"        FILES ${src_py_sdr_files})
source_group("encoders"   FILES ${src_py_encoders_files})
############################################################
#
# Build the shared binding libraries for the Python Interface
#
############################################################
#
# Note: if you need a debuggable version of the Python interface
#       See: https://bugs.python.org/issue22411
#       and: https://github.com/pybind/pybind11/issues/1295
#

# It is ready to use pybind11_add_module() macro.
# For details: https://pybind11.readthedocs.io/en/stable/compiling.html#cmake

set(algorithms_shared_lib algorithms)
pybind11_add_module(${algorithms_shared_lib} ${src_py_algorithms_files} )
target_link_libraries(${algorithms_shared_lib} PRIVATE
        ${htm_core_lib_path}
        ${COMMON_OS_LIBS}
        )
target_compile_options(${algorithms_shared_lib} PUBLIC ${INTERNAL_CXX_FLAGS})
target_compile_definitions(${algorithms_shared_lib} PRIVATE ${COMMON_COMPILER_DEFINITIONS})
target_include_directories(${algorithms_shared_lib} PRIVATE
        ${CORE_LIB_INCLUDES}
        ${CMAKE_CURRENT_SOURCE_DIR}
	    SYSTEM ${PYTHON_INCLUDE_DIRS}
	    SYSTEM ${pybind11_INCLUDE_DIRS}
        )

set(sdr_shared_lib sdr)
pybind11_add_module(${sdr_shared_lib} ${src_py_sdr_files} )
target_link_libraries(${sdr_shared_lib} PRIVATE
        ${htm_core_lib_path}
        ${COMMON_OS_LIBS}
        )
target_compile_options(${sdr_shared_lib} PUBLIC ${INTERNAL_CXX_FLAGS})
target_compile_definitions(${sdr_shared_lib} PRIVATE ${COMMON_COMPILER_DEFINITIONS})
target_include_directories(${sdr_shared_lib} PRIVATE
        ${CORE_LIB_INCLUDES}
        ${CMAKE_CURRENT_SOURCE_DIR}
        SYSTEM ${PYTHON_INCLUDE_DIRS}
        SYSTEM ${pybind11_INCLUDE_DIRS}
        SYSTEM ${EXTERNAL_INCLUDES}
	)


set(encoders_shared_lib encoders)
pybind11_add_module(${encoders_shared_lib} ${src_py_encoders_files} )
target_link_libraries(${encoders_shared_lib} PRIVATE
        ${htm_core_lib_path}
        ${COMMON_OS_LIBS}
        )
target_compile_options(${encoders_shared_lib} PUBLIC ${INTERNAL_CXX_FLAGS})
target_compile_definitions(${encoders_shared_lib} PRIVATE ${COMMON_COMPILER_DEFINITIONS})
target_include_directories(${encoders_shared_lib} PRIVATE
        ${CORE_LIB_INCLUDES}
        ${CMAKE_CURRENT_SOURCE_DIR}
        SYSTEM ${PYTHON_INCLUDE_DIRS}
        SYSTEM ${pybind11_INCLUDE_DIRS}
        SYSTEM ${EXTERNAL_INCLUDES}
	)

# Install the extension modules using scikit-build-core's install command
install(TARGETS ${sdr_shared_lib} ${encoders_shared_lib} ${algorithms_shared_lib}
        DESTINATION htm/bindings COMPONENT python)