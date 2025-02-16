# -----------------------------------------------------------------------------
# HTM Community Edition of NuPIC
# Copyright (C) 2021, Numenta, Inc.
# modified 4/4/2022 - newer version
# modified 12/12/2024 - use FetchContent, David Keeney dkeeney@gmail.com
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
# -----------------------------------------------------------------------------
#
# This will download the gtest module.
# exports 'gtest' as a target
#
include(FetchContent)
#

set(dependency_url "https://github.com/google/googletest/releases/download/v1.15.2/googletest-1.15.2.tar.gz")
set(local_override "${CMAKE_SOURCE_DIR}/build/Thirdparty/gtest")

# Check if local path exists and if so, use it as-is.
if(EXISTS ${local_override})
    message(STATUS "  Obtaining gtest from local override: ${local_override}")
    FetchContent_Declare(
        gtest
        SOURCE_DIR ${local_override}
        EXCLUDE_FROM_ALL
        QUIET
    )
else()
    message(STATUS "  Obtaining gtest from: ${dependency_url}")
    FetchContent_Declare(
        gtest
        URL ${dependency_url}
		    EXCLUDE_FROM_ALL
		    QUIET
    )
endif()

# Make sure that GTest does not try to install itself
set(INSTALL_GTEST OFF)
set(BUILD_GMOCK OFF)


FetchContent_MakeAvailable(gtest)

# For Windows: Prevent overriding the parent project's compiler/linker settings.
# This will force generation of a shared library for gtest.
if(MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

set_target_properties(gtest_main PROPERTIES EXCLUDE_FROM_ALL TRUE) # for gtest_main (not used)


set(gtest_INCLUDE_DIR "${gtest_SOURCE_DIR}/googletest/include")
