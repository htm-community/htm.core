# -----------------------------------------------------------------------------
# HTM Community Edition of NuPIC
# Copyright (C) 2020, Numenta, Inc.
# author: David Keeney, dkeeney@gmail.com, Feb 2020
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

# Fetch cpp-httplib package from GitHub archive
# This is a C++11 single-file header-only cross platform HTTP/HTTPS web server library.
# This is used only for the REST server example and not included in the htm.core library.
# header-only, so no build.
include(FetchContent)

set(dependency_url "https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.18.1.zip")
set(local_override "${CMAKE_SOURCE_DIR}/build/Thirdparty/cpp-httplib")

# Check if local path exists and if so, use it as-is.
if(EXISTS ${local_override})
    message(STATUS "  Obtaining cpp-httplib from local override: ${local_override}")
	
	# Note: FetchContent_Populate() will not execute it's CMakeLists.txt.  
	# This prevents it from building test and example code.
    FetchContent_Populate(
        cpp-httplib
        SOURCE_DIR ${local_override}
		QUIET
    )
else()
    message(STATUS "  Obtaining cpp-httplib from: ${dependency_url}  HEADER_ONLY")
	
	# Note: FetchContent_Populate() will download the package but will not execute it's 
	#       CMakeLists.txt.  This prevents it from building test and example code.
    FetchContent_Populate(
        cpp-httplib
        URL ${dependency_url}
        QUIET
    )
endif()


set(cpp-httplib_INCLUDE_DIR "${cpp-httplib_SOURCE_DIR}")


