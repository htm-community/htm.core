# -----------------------------------------------------------------------------
# HTM Community Edition of NuPIC
# Copyright (C) 2015-2018, Numenta, Inc.
#   Migrated to scikit-build-core:  David Keeney, dkeeney@gmail.com, Dec 2024
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

cmake_minimum_required(VERSION 3.21)

project(htm LANGUAGES C CXX)


set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
get_filename_component(REPOSITORY_DIR ${CMAKE_SOURCE_DIR} ABSOLUTE)
set_property(GLOBAL PROPERTY REPOSITORY_DIR "${REPOSITORY_DIR}")
cmake_policy(SET CMP0135 NEW)   # sets timestamp downloading along with dependancy download.

if(BINDING_BUILD STREQUAL "Python2")
  message(FATAL_ERROR "The Extension library cannot be built for Python 2 using MSVC.  Use Python 3.x")
endif()

message(STATUS "REPOSITORY_DIR       = ${REPOSITORY_DIR}")
message(STATUS "CMAKE_BINARY_DIR     = ${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_BUILD_TYPE     = ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")
message(STATUS "SKBUILD_DATA_DIR     = ${SKBUILD_DATA_DIR}")
message(STATUS "SKBUILD_PLATLIB_DIR  = ${SKBUILD_PLATLIB_DIR}")
message(STATUS "CMake Version:       = ${CMAKE_VERSION}")
message(STATUS "BINDING_BUILD        = ${BINDING_BUILD}")


IF (BINDING_BUILD STREQUAL "CPP_Only" )

    # define ${core_library} for the entire project.
    set(core_library   htm_core)
	if(MSVC)
	    set(src_lib_shared)
	else()
	    set(src_lib_shared htm_core_shared)
	endif()



	# get the project version from pyproject.toml and write it to the file VERSION.
	# NOTE: this is the VERSION in pyproject.toml. 
	#       copy this to the VERSION file in the project source.
	set(VERSION ${PROJECT_VERSION1})
	file(WRITE VERSION "${VERSION}") 

	message(STATUS "VERSION              = ${VERSION}")
	message("")

	# Determine common toolchain settings, compiler and link flags
	include(CommonCompilerConfig)

	message(STATUS "CMAKE_CXX_COMPILER_ID= ${CMAKE_CXX_COMPILER_ID}")
	message(STATUS "CMAKE_CXX_COMPILER_VERSION=${CMAKE_CXX_COMPILER_VERSION}")
	message(STATUS "CMAKE_CXX_COMPILER   = ${CMAKE_CXX_COMPILER}")
	message(STATUS "CMAKE_CXX_STANDARD   = ${CMAKE_CXX_STANDARD}")
	message(STATUS "PLATFORM             = ${PLATFORM}")
	message(STATUS "BITNESS              = ${BITNESS}")
	message(STATUS "INTERNAL_CXX_FLAGS   = ${INTERNAL_CXX_FLAGS}")
	message(STATUS "INTERNAL_LINKER_FLAGS= ${INTERNAL_LINKER_FLAGS}")
	message(STATUS "INTERNAL_LINKER_FLAGS_STR= ${INTERNAL_LINKER_FLAGS_STR}")
	message(STATUS "CMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}")
	message(STATUS "COMMON_COMPILER_DEFINITIONS_STR=${COMMON_COMPILER_DEFINITIONS_STR}")
	message(STATUS "COMMON_OS_LIBS       = ${COMMON_OS_LIBS}")



	# Set up builds of external dependencies and get their exports.
	# (see individual external/*.cmake modules for exported settings and functions)
	include(external/bootstrap.cmake)
 


	# Build the C++ library
	add_subdirectory(src)
else()

	message(STATUS "VERSION              = ${VERSION}")
	# Determine common toolchain settings, compiler and link flags
	include(CommonCompilerConfig)
	set(HTM_CORE_LIB_DIR "${CMAKE_SOURCE_DIR}/build/Release/lib" CACHE PATH "Path to the built htm_core library")
    list(APPEND CORE_LIB_INCLUDES "${CMAKE_SOURCE_DIR}/build/Release/include")

	message(STATUS "INTERNAL_CXX_FLAGS   = ${INTERNAL_CXX_FLAGS}")
	message(STATUS "INTERNAL_LINKER_FLAGS= ${INTERNAL_LINKER_FLAGS}")
    message(STATUS "HTM_CORE_LIB_DIR     = ${HTM_CORE_LIB_DIR}") # Add this line

	# Build the Python extension interface using pybind11.
	# Add subdirectory for Python bindings
	add_subdirectory(bindings/py/cpp_src)
endif()


