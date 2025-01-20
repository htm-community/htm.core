# -----------------------------------------------------------------------------
# HTM Community Edition of NuPIC
# Copyright (C) 2016, Numenta, Inc.
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

# Download the MNIST dataset from online archive
# This is used for the htm.core examples and tests.
#
include(FetchContent)

set(dependency_url "https://github.com/wichtounet/mnist/archive/3b65c35ede53b687376c4302eeb44fdf76e0129b.zip")
set(local_override "${CMAKE_SOURCE_DIR}/build/Thirdparty/mnist")

# Check if local path exists and if so, use it as-is.
if(EXISTS ${local_override})
    message(STATUS "  Obtaining mnist from local override: ${local_override}")
    FetchContent_Populate(
        mnist
        SOURCE_DIR ${local_override}
		QUIET
    )
else()
    message(STATUS "  Obtaining mnist from: ${dependency_url}")
    FetchContent_Populate(
        mnist
        URL ${dependency_url}
		QUIET
    )
	# Copy the MNIST data to the official location.
	file(COPY "${mnist_SOURCE_DIR}/" 
	         DESTINATION "${CMAKE_SOURCE_DIR}/build/Thirdparty/mnist")
endif()


# The data files are actually in the source folder of the archive.
# There is a CMakeLists.txt but it is in the example subfolder.
#
# Includes will be found with  #include <mnist/mnist_reader.hpp>
# Data will be found in folder "build/Thirdparty/mnist"
# The executable in build/Release/bin, will find the data at "../../Thirdparty/mnist"
# 
set(mnist_INCLUDE_DIR "${mnist_SOURCE_DIR}/include")
