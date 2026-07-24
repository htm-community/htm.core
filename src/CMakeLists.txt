# -----------------------------------------------------------------------------
# HTM Community Edition of NuPIC
# Copyright (C) 2013-2015, Numenta, Inc.
#   Migrated to scikit-build-core:  David Keeney, dkeeney@gmail.com, Dec 2024
#   Trimmed for PyHTM-core: only the algorithm/encoder/SDR engine is built.
#     Removed: engine/ (Network API), regions/, ntypes/, RESTapi, SDRClassifier,
#     SimHashDocumentEncoder and their external deps (libyaml, sqlite3,
#     cpp-httplib, digestpp, eigen, mnist, gtest). PyHTM drives SP/TM/SDR/RDSE
#     directly through the Python bindings, so none of that code ever ran.
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

cmake_minimum_required(VERSION 3.21)
project(htm_core LANGUAGES CXX)

message(STATUS "Configuring htm_core src")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_VERBOSE_MAKEFILE ON) # toggle for cmake debug

#
# NuPIC CMake options
#
option(HTM_IWYU "Enable include-what-you-use
  (http://include-what-you-use.org/). This requires the iwyu binary to be
  discoverable by CMake's find_program, with a minimum CMake version of 3.3.")
if(${HTM_IWYU})
  find_program(iwyu_path NAMES include-what-you-use iwyu)
  if(NOT iwyu_path)
    message(FATAL_ERROR "Could not find the program include-what-you-use")
  endif()
endif()

#--------------------------------------------------------
# Identify includes from this directory
set(CORE_LIB_INCLUDES  ${PROJECT_SOURCE_DIR}  # for htm/xxx/*.h
		       ${PROJECT_BINARY_DIR}  # for Version.hpp
)
set(CORE_LIB_INCLUDES  ${CORE_LIB_INCLUDES} PARENT_SCOPE)


#--------------------------------------------------------
# Create Version.hpp
# Extract current git sha and record in htm/Version.hpp.in
#
set(HTM_CORE_VERSION ${VERSION})
configure_file(
  "${PROJECT_SOURCE_DIR}/htm/Version.hpp.in"
  "${PROJECT_BINARY_DIR}/Version.hpp")


############ htm_core Library ######################################
# Setup htm_core static library from our own sources.
#
# this is an intermediate archive that will be merged with external static
# libraries in a subsequent step to produce the htm_core "combined" static
# library.
#

set(src_objlib LibrarySource)


set(algorithm_files
    htm/algorithms/Anomaly.cpp
    htm/algorithms/Anomaly.hpp
    htm/algorithms/AnomalyLikelihood.cpp
    htm/algorithms/AnomalyLikelihood.hpp
    htm/algorithms/Connections.cpp
    htm/algorithms/Connections.hpp
    htm/algorithms/SpatialPooler.cpp
    htm/algorithms/SpatialPooler.hpp
    htm/algorithms/TemporalMemory.cpp
    htm/algorithms/TemporalMemory.hpp
)


set(encoders_files
    htm/encoders/BaseEncoder.hpp
    htm/encoders/DateEncoder.cpp
    htm/encoders/DateEncoder.hpp
    htm/encoders/ScalarEncoder.cpp
    htm/encoders/ScalarEncoder.hpp
    htm/encoders/RandomDistributedScalarEncoder.hpp
    htm/encoders/RandomDistributedScalarEncoder.cpp
)

set(os_files
    htm/os/Directory.cpp
    htm/os/Directory.hpp
    htm/os/ImportFilesystem.hpp
    htm/os/Path.cpp
    htm/os/Path.hpp
)

set(types_files
    htm/types/Exception.hpp
    htm/types/Types.hpp
    htm/types/Serializable.hpp
    htm/types/Sdr.hpp
    htm/types/Sdr.cpp
)

set(utils_files
    htm/utils/Log.cpp
    htm/utils/Log.hpp
    htm/utils/MovingAverage.cpp
    htm/utils/MovingAverage.hpp
    htm/utils/Random.cpp
    htm/utils/Random.hpp
    htm/utils/SlidingWindow.hpp
    htm/utils/VectorHelpers.hpp
    htm/utils/SdrMetrics.cpp
    htm/utils/SdrMetrics.hpp
    htm/utils/Topology.cpp
    htm/utils/Topology.hpp
)


#set up file tabs in Visual Studio
source_group("algorithms" FILES ${algorithm_files})
source_group("encoders" FILES ${encoders_files})
source_group("os" FILES ${os_files})
source_group("types" FILES ${types_files})
source_group("utils" FILES ${utils_files})


#--------------------------------------------------------
# Create an OBJECT library (compiles sources only once)
# from the CMake docs:
#   An object library compiles source files but does not archive or link their object 
#   files into a library. Instead other targets created by  add_library() or add_executable() 
#   may reference the objects using an expression of the form $<TARGET_OBJECTS:objlib> as a 
#   source, where objlib is the object library name.
#
# The price you pay is that the object files must be built as position-independent code.
#
# Integer AVX-512 histogram kernel (vpconflictd + gather/scatter) in
# Connections.cpp: enable F+CD for THIS file only, on top of the library's
# v3 cap. The kernel is integer-only (order-independent => bit-exact by
# construction) and compiles in ONLY when the build host actually detected
# level 4, so a v2/v3 machine never receives AVX-512 instructions.
if(NOT MSVC AND DEFINED HTM_MARCH_DETECTED AND HTM_MARCH_DETECTED GREATER 3)
        message(STATUS "Connections.cpp: +avx512f,cd (integer histogram kernel)")
        set_source_files_properties(htm/algorithms/Connections.cpp
                PROPERTIES COMPILE_OPTIONS "-mavx512f;-mavx512cd")
endif()

add_library(${src_objlib} OBJECT 
    ${algorithm_files} 
    ${encoders_files} 
    ${os_files} 
    ${types_files} 
    ${utils_files}
)
# shared libraries need PIC
target_compile_options( ${src_objlib} PUBLIC ${INTERNAL_CXX_FLAGS})
set_target_properties(${src_objlib} PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MSVC)
  set_property(TARGET ${src_objlib} PROPERTY LINK_LIBRARIES ${INTERNAL_LINKER_FLAGS})
endif()
target_compile_definitions(${src_objlib} PRIVATE ${COMMON_COMPILER_DEFINITIONS})
target_include_directories(${src_objlib} PRIVATE 
		${CORE_LIB_INCLUDES} 
		${EXTERNAL_INCLUDES}
		)

if(${HTM_IWYU})
  set_target_properties(${src_objlib}
    PROPERTIES CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})
endif()

############ Building Static LIB #############################################
# build static htm_core.a with the common (MurmurHash3) objects.
# uses objects compiled for src_objlib
set(EXTRA_LIBS
    $<TARGET_OBJECTS:${common_TARGET}>
)

add_library(${core_library} STATIC $<TARGET_OBJECTS:${src_objlib}> ${EXTRA_LIBS})

if(MSVC)
  set_property(TARGET ${core_library} PROPERTY LINK_LIBRARIES ${INTERNAL_LINKER_FLAGS})
endif()

############ Building Shared LIB #############################################
# builds libhtm_core.so for Linux,OSx and 
# NOTE: This does not work for Windows.
#       If there is community demand for a Windows DLL interface, we can add it later.
#
# For Linux, OSx; the .so file will contain all symbols.
#
if(MSVC)
  # NOTE: this disables shared lib for Windows.
else()
  add_library(${src_lib_shared} SHARED
      $<TARGET_OBJECTS:${src_objlib}>
  )
  target_link_libraries(${src_lib_shared}
      	PUBLIC
	    ${common_TARGET}
  )
  set_target_properties(${src_lib_shared} PROPERTIES OUTPUT_NAME ${core_library})
endif()


############ INSTALL ######################################
#
# Install C++ targets into CMAKE_INSTALL_PREFIX if doing a C++ library build.
install(TARGETS
        ${core_library}
        DESTINATION ${CMAKE_SOURCE_DIR}/build/Release/lib)

if(MSVC)
else()
  install(TARGETS
        ${src_lib_shared}
        RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/build/Release/bin
        LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/build/Release/lib
        ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/build/Release/lib)
endif()

# Version.hpp is also used by the htm.bindings release/deployment system
install(FILES ${PROJECT_BINARY_DIR}/Version.hpp
      DESTINATION ${CMAKE_SOURCE_DIR}/build/Release/include)

# Install the source include files
install(DIRECTORY ${PROJECT_SOURCE_DIR}/htm
      MESSAGE_NEVER
      DESTINATION ${CMAKE_SOURCE_DIR}/build/Release/include
      FILES_MATCHING PATTERN "*.h*" PATTERN "*.hpp.in" EXCLUDE)

# Install the external dependency include files
foreach(dir ${EXTERNAL_INCLUDES})
    install(DIRECTORY "${dir}/"
	    MESSAGE_NEVER
        DESTINATION "${CMAKE_SOURCE_DIR}/build/Release/include"
        FILES_MATCHING PATTERN "*.h*" PATTERN "*.hpp" PATTERN "*.inl")
endforeach()

install(FILES 
    ${CMAKE_SOURCE_DIR}/LICENSE.txt  
    ${CMAKE_SOURCE_DIR}/README.md  
    ${CMAKE_SOURCE_DIR}/VERSION  
    DESTINATION ${CMAKE_SOURCE_DIR}/build/Release
)
