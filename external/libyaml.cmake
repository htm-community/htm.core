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

# This downloads and builds the libyaml library.
#

# libyaml  - This is a SAX parser which means that it performs callbacks
#            or returns tokens for each symbol it parses from the yaml text.  
#            Therefore the interface (Value.cpp) must create the internal structure.
#            This is a YAML 1.1 implementation.
#
#            The repository is at https://github.com/yaml/libyaml
#            Documentation is at https://pyyaml.org/wiki/LibYAML
#
include(FetchContent)
set(dependency_url "https://github.com/yaml/libyaml/archive/refs/tags/0.2.5.tar.gz")
set(local_override "${CMAKE_SOURCE_DIR}/build/Thirdparty/libyaml")



# Check if local path exists and if so, use it as-is.
if(EXISTS ${local_override})
    message(STATUS "  Obtaining libyaml from local override: ${local_override}")
    FetchContent_Populate(
        libyaml
        SOURCE_DIR ${local_override}
		QUIET
    )
else()
    message(STATUS "  Obtaining libyaml from: ${dependency_url}")
    FetchContent_Populate(
        libyaml
        URL ${dependency_url}
		QUIET
    )
endif()

# Create config.h with the appropriate definitions
set(config_h "${libyaml_BINARY_DIR}/include/config.h")
file(MAKE_DIRECTORY "${libyaml_BINARY_DIR}/include") # Ensure directory exists

file(WRITE  "${config_h}" "#ifndef YAML_CONFIG_H_INCLUDED\n")
file(APPEND "${config_h}" "  #define YAML_CONFIG_H_INCLUDED\n\n")
file(APPEND "${config_h}" "  #define YAML_VERSION_MAJOR 0\n")
file(APPEND "${config_h}" "  #define YAML_VERSION_MINOR 2\n")
file(APPEND "${config_h}" "  #define YAML_VERSION_PATCH 5\n")
file(APPEND "${config_h}" "  #define YAML_VERSION_STRING \"0.2.5\"\n\n")
file(APPEND "${config_h}" "#endif /* YAML_CONFIG_H_INCLUDED */\n")


# Create a custom object library target for libyaml
add_library(yaml_obj OBJECT)
target_sources(yaml_obj
    PRIVATE
        "${libyaml_SOURCE_DIR}/src/api.c"
        "${libyaml_SOURCE_DIR}/src/dumper.c"
        "${libyaml_SOURCE_DIR}/src/emitter.c"
        "${libyaml_SOURCE_DIR}/src/loader.c"
        "${libyaml_SOURCE_DIR}/src/parser.c"
        "${libyaml_SOURCE_DIR}/src/reader.c"
        "${libyaml_SOURCE_DIR}/src/scanner.c"
        "${libyaml_SOURCE_DIR}/src/writer.c"
)


target_compile_definitions(yaml_obj PRIVATE YAML_DECLARE_STATIC HAVE_CONFIG_H)
# shared libraries need PIC
if(MSVC)
  target_compile_options( yaml_obj PUBLIC ${INTERNAL_CXX_FLAGS}  /wd4267 /wd4244)
  set_property(TARGET yaml_obj PROPERTY LINK_LIBRARIES ${INTERNAL_LINKER_FLAGS})
else()
  target_compile_options( yaml_obj PUBLIC  -Wunused-value
		-Wno-conversion -Wno-sign-conversion -Wno-unused-value
		-fdiagnostics-show-option -fPIC -Wextra -Wreturn-type 
                -Wunused -Wno-unused-variable -Wno-unused-parameter  
		-Wno-missing-field-initializers  -Wall -pipe -O3 -mtune=generic)
endif()
set_target_properties(yaml_obj PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(yaml_obj PRIVATE  ${COMMON_COMPILER_DEFINITIONS})
target_include_directories(yaml_obj PUBLIC  "${libyaml_SOURCE_DIR}/include")
target_include_directories(yaml_obj PRIVATE "${libyaml_BINARY_DIR}/include")

# Set variables for other parts of your CMake project to use
set(libyaml_INCLUDE_DIR "${libyaml_SOURCE_DIR}/include")
set(libyaml_TARGET yaml_obj) # Use the object library target
