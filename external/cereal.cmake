# -----------------------------------------------------------------------------
# HTM Community Edition of NuPIC
# Copyright (C) 2016, Numenta, Inc.
# modified 4/4/2022 - newer version of cerial
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
# along with this program.  If not, see http://www.gnu.org/licenses.
# -----------------------------------------------------------------------------

# Get the Cereal serialization package from GitHub archive
#     https://uscilab.github.io/cereal/
#
# This file downloads and configures the cereal external component.
# No build. This is a header only package
include(FetchContent)


set(dependency_url "https://github.com/USCiLab/cereal/archive/refs/tags/v1.3.2.tar.gz")
set(local_override "${CMAKE_SOURCE_DIR}/build/Thirdparty/cereal")


# Check if local path exists and if so, use it as-is.
if(EXISTS ${local_override})
    message(STATUS "  Obtaining cereal from local override: ${local_override}  HEADER_ONLY")
	
	# Note: FetchContent_Populate() will not execute it's CMakeLists.txt.  
	# This prevents it from building test and example code.
    FetchContent_Populate(
        cereal
        SOURCE_DIR ${local_override}
		QUIET
    )
else()
    message(STATUS "  Obtaining cereal from: ${dependency_url}  HEADER_ONLY")
	
	# Note: FetchContent_Populate() will download the package but will not execute it's 
	#       CMakeLists.txt.  This prevents it from building test and example code.
    FetchContent_Populate(
        cereal
        URL ${dependency_url}
		QUIET
    )
endif()


	    

# access as #include "cereal"
set(cereal_INCLUDE_DIR "${cereal_SOURCE_DIR}/include")
set(cereal_INCLUDE_DIRS "${cereal_SOURCE_DIR}/include")  #for install
	



