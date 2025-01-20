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
#######################################
# This bootstrap cmake file is used to start the ThirdParty builds.
# Note: You can override this download (like for air-gap computers) by manually obtaining
#       the distribution package and manually expanding it in the folders
#             build/Thirdparty/<dependency_name>


include(external/common.cmake)          # Build instructions for the common library
include(external/cereal.cmake)          # Build instructions for cereal
include(external/cpp-httplib.cmake)     # Build instructions for cpp-httplib
include(external/digestpp.cmake)        # Build instructions for digestpp
include(external/eigen.cmake)           # Build instructions for Eigen
include(external/gtest.cmake)           # Build instructions for gtest
include(external/libyaml.cmake)         # Build instructions for libyaml
include(external/mnist.cmake)           # Build instructions for mnist
include(external/sqlite3.cmake)         # Build instructions for sqlite3


# Define EXTERNAL_INCLUDES after including bootstrap.cmake
set(EXTERNAL_INCLUDES "")
list(APPEND EXTERNAL_INCLUDES "${cereal_INCLUDE_DIR}")
list(APPEND EXTERNAL_INCLUDES "${eigen_INCLUDE_DIR}")
list(APPEND EXTERNAL_INCLUDES "${cpp-httplib_INCLUDE_DIR}")
list(APPEND EXTERNAL_INCLUDES "${digestpp_INCLUDE_DIR}")
list(APPEND EXTERNAL_INCLUDES "${gtest_INCLUDE_DIR}")
list(APPEND EXTERNAL_INCLUDES "${libyaml_INCLUDE_DIR}")
list(APPEND EXTERNAL_INCLUDES "${mnist_INCLUDE_DIR}")
list(APPEND EXTERNAL_INCLUDES "${sqlite3_INCLUDE_DIR}")
list(APPEND EXTERNAL_INCLUDES "${common_INCLUDE_DIR}")


