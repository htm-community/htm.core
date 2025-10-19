# -----------------------------------------------------------------------------
# HTM Community Edition of NuPIC
# Copyright (C) 2019, Numenta, Inc.
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
# along with this program.� If not, see http://www.gnu.org/licenses.
# -----------------------------------------------------------------------------

# Download digestpp hash digest package from GitHub archive
# This file downloads and configures the digestpp library.
#  header-only, no build.
include(FetchContent)

set(dependency_url "https://github.com/kerukuro/digestpp/archive/refs/heads/master.zip")
set(local_override "${CMAKE_SOURCE_DIR}/build/Thirdparty/digestpp")

# Check if local path exists and if so, use it as-is.
if(EXISTS ${local_override})
    message(STATUS "  Obtaining digestpp from local override: ${local_override}")
    FetchContent_Populate(
        digestpp
        SOURCE_DIR ${local_override}
		QIIET
    )
else()
    message(STATUS "  Obtaining digestpp from: ${dependency_url}  HEADER_ONLY")
    FetchContent_Populate(
        digestpp
        URL ${dependency_url}
		QUIET
    )
endif()

# This does not have a CMakeList.txt and does not build tests or examples.

# access with #include "digestpp.h"
set(digestpp_INCLUDE_DIR "${digestpp_SOURCE_DIR}")


