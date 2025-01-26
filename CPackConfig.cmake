# CPackConfig.cmake
# This is used to package the htm_core library for use with C++ applications.

# Set the package name and version
set(CPACK_PACKAGE_NAME "htm_core_cpp")  # Choose a name for your C++ package
set(CPACK_PACKAGE_VERSION "2.2.0")      # Update with your current version

# Set other package properties (optional)
set(CPACK_PACKAGE_VENDOR "HTM Community")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "HTM Core C++ Library")

# Create a component for the mnist data
set(CPACK_COMPONENTS_ALL mnist examples)  # List all components
set(CPACK_COMPONENT_mnist_DISPLAY_NAME "MNIST Example Data")
set(CPACK_COMPONENT_mnist_DESCRIPTION "Data files for the MNIST examples")
set(CPACK_COMPONENT_mnist_GROUP "Example Data")

############### PACKAGE ###########################
#
# `make package` results in
# htm_cpp-${VERSION}-${PLATFORM}${BITNESS}${PLATFORM_SUFFIX}.tar.gz binary release
#
#   
string(REGEX REPLACE "^([vV])([0-9]*)([.][0-9]*[.][0-9]*-?.*)$" "\\2" numbers ${CPACK_PACKAGE_VERSION} )
set(MAJOR ${numbers})
string(REGEX REPLACE "^([vV][0-9]*[.])([0-9]*)([.][0-9]*-?.*)$" "\\2" numbers ${CPACK_PACKAGE_VERSION} )
set(MINOR ${numbers})
string(REGEX REPLACE "^([vV][0-9]*[.][0-9]*[.])([0-9]*)(-?.*)$" "\\2" numbers ${CPACK_PACKAGE_VERSION} )
set(PATCH ${numbers})

message(STATUS "Packaging version: ${CPACK_PACKAGE_VERSION} in CPACK")
set(CPACK_PACKAGE_NAME "htm.core")
set(CPACK_PACKAGE_VENDOR "https://github.com/htm-community/htm.core")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "htm.core - Community implementation of Numenta's algorthms.")
set(CPACK_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PATCH})

if(MSVC)
	set(CPACK_GENERATOR "ZIP")
else()
	set(CPACK_GENERATOR "TGZ")
endif()
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_FILE_NAME "htm_core-${CPACK_PACKAGE_VERSION}-${PLATFORM}${BITNESS}${PLATFORM_SUFFIX}")
include(CPack)