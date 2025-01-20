# -----------------------------------------------------------------------------
# HTM Community Edition of NuPIC
# Copyright (C) 2016, 2024, Numenta, Inc.
#		Modified by David Keeney, dkeeney@gmail.com Dec 2024
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
# This creates a library from external source code that could not be downloaded
# so it is physically included in this repository.
#
# Directions for adding a package:
# 1) If it is an header-only package, then just add it to external/include and skip the rest of these directions.
# 2) Create a folder for the package in external/common/ and copy all files there.
# 3) Add the .cpp files to the common_src list.
# 4) The include files will be accessable with a path starting with the name of the folder created for your package.
# 5) The objects will be placed in common.lib and included as part of the external libraries.
#
# Notes about Common
#   #include <murmurhash3/MurmurHash3.hpp>
#   #include "csv.h"
#   link with common.lib
#
#   File layout in external/common
#       common
#          |--- murmurhash3
#          |       |----MurmurHash3.cpp
#          |
#          |--- include
#                  |--- csv.h
#                  |--- CSV_README.md
#                  |--- murmurhash3
#                         |----MurmurHash3.hpp
#
message(STATUS "  Obtaining common")
get_property(REPOSITORY_DIR GLOBAL PROPERTY REPOSITORY_DIR)

# Define source and include directories
set(common_SOURCE_DIR ${REPOSITORY_DIR}/external/common)

# Add MurmurHash3 source
set(common_src 
    "${common_SOURCE_DIR}/murmurhash3/MurmurHash3.cpp"
	)
	
source_group("common" FILES ${common_src})

# Build the static library
add_library(common OBJECT ${common_src})
target_compile_definitions(common PRIVATE ${COMMON_COMPILER_DEFINITIONS})
set_target_properties(common PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(common PUBLIC "${common_SOURCE_DIR}/include")
set(common_INCLUDE_DIR  ${common_SOURCE_DIR}/include)
	
set(common_TARGET common)



