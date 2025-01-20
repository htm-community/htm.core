# -----------------------------------------------------------------------------
# Numenta Platform for Intelligent Computing (NuPIC)
# Copyright (C) 2019, Numenta, Inc.  
# modified 4/4/2022 - newer version
# modified 12/12/2024 - use FetchContent, David Keeney dkeeney@gmail.com
#
# Unless you have purchased from
# Numenta, Inc. a separate commercial license for this software code, the
# following terms and conditions apply:
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
# along with this program.  If not, see http://www.gnu.org/licenses.
#
# http://numenta.org/licenses/
# -----------------------------------------------------------------------------

# This downloads and builds the sqlite3 library.
#

# SQLite is a C-language library that implements a small, fast, self-contained, 
#        high-reliability, full-featured, SQL database engine. SQLite is the most used 
#        database engine in the world.
#
include(FetchContent)

set(dependency_url "https://www.sqlite.org/2024/sqlite-amalgamation-3470000.zip")
set(local_override "${CMAKE_SOURCE_DIR}/build/Thirdparty/sqlite3")

# Check if local path exists and if so, use it as-is.
if(EXISTS ${local_override})
    message(STATUS "  Obtaining sqlite3 from local override: ${local_override}")
    FetchContent_Populate(
        sqlite3
        SOURCE_DIR ${local_override}
		QUIET
    )
else()
    message(STATUS "  Obtaining sqlite3 from: ${dependency_url}")
    FetchContent_Populate(
        sqlite3
        URL ${dependency_url}
		QUIET
    )
endif()


# SQLite does not provide a CMakeList.txt to build with so we provide the following 
# lines to perform the compile here.
# Reference the include as #include "sqlite3.h".

set(sqlite3_srcs "${sqlite3_SOURCE_DIR}/sqlite3.c")
set(sqlite3_hdrs "${sqlite3_SOURCE_DIR}/sqlite3.h")

add_library(sqlite3 OBJECT ${sqlite3_srcs} ${sqlite3_hdrs})
target_compile_definitions(sqlite3 PRIVATE ${COMMON_COMPILER_DEFINITIONS})
set_target_properties(sqlite3 PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(sqlite3 PUBLIC "${sqlite3_SOURCE_DIR}")
set(sqlite3_INCLUDE_DIR "${sqlite3_SOURCE_DIR}")

set(sqlite3_TARGET sqlite3)
