# -----------------------------------------------------------------------------
# HTM Community Edition of NuPIC
# Copyright (C) 2013-2015, Numenta, Inc.
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

cmake_minimum_required(VERSION 3.7)
project(htm_core CXX)

message(STATUS "Configuring htm_core src")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_VERBOSE_MAKEFILE OFF) # toggle for cmake debug
include(src/NupicLibraryUtils) # for combining static libs
include(src/ExpandStaticLib)   # for creating objects from a static lib

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
# Setup htm_core static library, consisting of our own sources and yaml libraries;
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
    htm/algorithms/SDRClassifier.cpp
    htm/algorithms/SDRClassifier.hpp
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
    htm/encoders/SimHashDocumentEncoder.hpp
    htm/encoders/SimHashDocumentEncoder.cpp
)
    
set(engine_files
    htm/engine/Input.cpp
    htm/engine/Input.hpp
    htm/engine/Link.cpp
    htm/engine/Link.hpp
    htm/engine/Network.cpp
    htm/engine/Network.hpp
    htm/engine/Output.cpp
    htm/engine/Output.hpp
    htm/engine/Region.cpp
    htm/engine/Region.hpp
    htm/engine/RegionImpl.cpp
    htm/engine/RegionImpl.hpp
    htm/engine/RegionImplFactory.cpp
    htm/engine/RegionImplFactory.hpp
    htm/engine/RegisteredRegionImpl.hpp
    htm/engine/RegisteredRegionImplCpp.hpp
    htm/engine/RESTapi.hpp
    htm/engine/RESTapi.cpp
    htm/engine/RawInput.hpp
    htm/engine/Spec.cpp
    htm/engine/Spec.hpp
    htm/engine/Watcher.cpp
    htm/engine/Watcher.hpp
)


set(ntypes_files
    htm/ntypes/Array.hpp
    htm/ntypes/ArrayBase.cpp
    htm/ntypes/ArrayBase.hpp
    htm/ntypes/BasicType.cpp
    htm/ntypes/BasicType.hpp
    htm/ntypes/Collection.hpp
    htm/ntypes/Dimensions.hpp
    htm/ntypes/Value.cpp
    htm/ntypes/Value.hpp
)
  
set(os_files
    htm/os/Directory.cpp
    htm/os/Directory.hpp
    htm/os/Env.cpp
    htm/os/Env.hpp
    htm/os/ImportFilesystem.hpp
    htm/os/Path.cpp
    htm/os/Path.hpp
    htm/os/Timer.cpp
    htm/os/Timer.hpp    
)

set(regions_files
    htm/regions/DateEncoderRegion.cpp
    htm/regions/DateEncoderRegion.hpp    
    htm/regions/ClassifierRegion.cpp
    htm/regions/ClassifierRegion.hpp
    htm/regions/ScalarEncoderRegion.cpp
    htm/regions/ScalarEncoderRegion.hpp    
    htm/regions/RDSEEncoderRegion.cpp
    htm/regions/RDSEEncoderRegion.hpp
    htm/regions/SPRegion.cpp
    htm/regions/SPRegion.hpp
    htm/regions/TestNode.cpp
    htm/regions/TestNode.hpp
    htm/regions/TMRegion.cpp
    htm/regions/TMRegion.hpp
    htm/regions/VectorFile.cpp
    htm/regions/VectorFile.hpp
    htm/regions/FileOutputRegion.cpp
    htm/regions/FileOutputRegion.hpp
    htm/regions/FileInputRegion.cpp
    htm/regions/FileInputRegion.hpp  
    htm/regions/DatabaseRegion.cpp
    htm/regions/DatabaseRegion.hpp
)

set(types_files
    htm/types/Exception.hpp
    htm/types/Types.hpp
    htm/types/Serializable.hpp
    htm/types/Sdr.hpp
    htm/types/Sdr.cpp
)

set(utils_files
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

set(examples_files
    examples/hello/hello.cpp # contains conflicting main()
    examples/hello/HelloSPTP.cpp
    examples/hello/HelloSPTP.hpp
    examples/napi_hello/napi_hello.cpp
    examples/napi_hello/napi_hello_database.cpp
    examples/mnist/MNIST_SP.cpp
    examples/rest/server_core.hpp
    examples/rest/server.cpp
    examples/rest/client.cpp
)


#set up file tabs in Visual Studio
source_group("algorithms" FILES ${algorithm_files})
source_group("encoders" FILES ${encoders_files})
source_group("engine" FILES ${engine_files})
source_group("ntypes" FILES ${ntypes_files})
source_group("os" FILES ${os_files})
source_group("regions" FILES ${regions_files})
source_group("types" FILES ${types_files})
source_group("utils" FILES ${utils_files})
source_group("examples" FILES ${examples_files})


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
add_library(${src_objlib} OBJECT 
    ${algorithm_files} 
    ${encoders_files} 
    ${engine_files} 
    ${ntypes_files} 
    ${os_files} 
    ${regions_files} 
    ${types_files} 
    ${utils_files}
)
# shared libraries need PIC
target_compile_options( ${src_objlib} PUBLIC ${INTERNAL_CXX_FLAGS})
if(MSVC)
  set_property(TARGET ${src_objlib} PROPERTY LINK_LIBRARIES ${INTERNAL_LINKER_FLAGS})
endif()
target_compile_definitions(${src_objlib} PRIVATE ${COMMON_COMPILER_DEFINITIONS})
target_compile_definitions(${src_objlib} PRIVATE ${yaml_DEFINE})
target_include_directories(${src_objlib} PRIVATE 
		${CORE_LIB_INCLUDES} 
		SYSTEM ${EXTERNAL_INCLUDES}
		)

if(${HTM_IWYU})
  set_target_properties(${src_objlib}
    PROPERTIES CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})
endif()

############ Building Static LIB #############################################
# build static libhtm_core_solo.a without yaml and boost.
# uses objects compiled for src_objlib

set(src_lib_static htm_core_solo)
add_library(${src_lib_static} STATIC $<TARGET_OBJECTS:${src_objlib}>)
if(MSVC)
  set_property(TARGET ${src_lib_static} PROPERTY LINK_LIBRARIES ${INTERNAL_LINKER_FLAGS})
endif()
#
# Create the libhtm_core.a as a "combined" static library by merging htm_core_static
# with our external static libraries
# NOTE: cannot use target_link_libraries( ) with the add_library for STATIC.
#       so we have to merge them in a different way.
#
set(src_combined_htmcore_source_archives
    ${src_lib_static}
    ${yaml_LIBRARIES}
    ${sqlite3_LIBRARIES}
    ${Boost_LIBRARIES}
    ${common_LIBRARIES}
    )
merge_static_libraries(${core_library} "${src_combined_htmcore_source_archives}")

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
    # First extract static libraries into an Object Library
	# Note that these libraries must already exist (from depenancies)
	set(all_object_locations)
	set(src_externLibs
            ${yaml_LIBRARIES}
            ${sqlite3_LIBRARIES}
            ${Boost_LIBRARIES}
            ${common_LIBRARIES}
            )
	set(lib_locations_separator "++++")
    string(REPLACE ";" ${lib_locations_separator}
           escaped_lib_locations_arg "${src_externLibs}")

	ExpandStaticLib( ${src_lib_shared}
	    "${escaped_lib_locations_arg}"
	    "${lib_locations_separator}"
	    "${CMAKE_CURRENT_BINARY_DIR}"
	    "${CMAKE_AR}"
	    )
	# build shared lib with yaml and boost resolved
	# uses objects compiled for src_objlib
	add_library(${src_lib_shared} SHARED 
		$<TARGET_OBJECTS:${src_objlib}>
		${all_object_locations})
	set_target_properties(${src_lib_shared} PROPERTIES OUTPUT_NAME ${core_library})
	target_link_libraries(${src_lib_shared}  
		${COMMON_OS_LIBS}
	        )
endif()



		
############ TEST #############################################
# Test
# The tests were placed in a separate file to avoid clutering up this file.
add_subdirectory(test)


########### EXAMPLES ##############################################
## Example custom executable
#
## Setup hello
#
set(src_executable_hello hello)
add_executable(${src_executable_hello} examples/hello/hello.cpp examples/hello/HelloSPTP.hpp examples/hello/HelloSPTP.cpp)
# link with the static library
target_link_libraries(${src_executable_hello} 
    ${INTERNAL_LINKER_FLAGS}
    ${core_library}
    ${COMMON_OS_LIBS}
)
    
# for future reference; to link to shared library, (Not on MSVC)
#          link with ${src_lib_shared} rather than ${core_library}
                  
target_compile_options( ${src_executable_hello} PUBLIC ${INTERNAL_CXX_FLAGS})
target_compile_definitions(${src_executable_hello} PRIVATE ${COMMON_COMPILER_DEFINITIONS})
target_include_directories(${src_executable_hello} PRIVATE 
		${CORE_LIB_INCLUDES} 
		SYSTEM ${EXTERNAL_INCLUDES}
		)

###add_custom_target(benchmark_hello
###          COMMAND ${src_executable_hello}
###          DEPENDS ${src_executable_hello}
###          COMMENT "Executing ${src_executable_hello}"
###          VERBATIM)

#########################################################
## Dynamicly linked version of hello
if(MSVC)
else()
  set(src_dyn_executable_hello dynamic_hello)
  add_executable(${src_dyn_executable_hello} examples/hello/hello.cpp examples/hello/HelloSPTP.hpp examples/hello/HelloSPTP.cpp)
  # for Linux,OSx, link with the shared library
  target_link_libraries(${src_dyn_executable_hello} 
    ${INTERNAL_LINKER_FLAGS}
    ${src_lib_shared}
    ${COMMON_OS_LIBS}
  )
  target_compile_options( ${src_dyn_executable_hello} PUBLIC ${INTERNAL_CXX_FLAGS})
  target_compile_definitions(${src_dyn_executable_hello} PRIVATE ${COMMON_COMPILER_DEFINITIONS})
  target_include_directories(${src_dyn_executable_hello} PRIVATE 
		${CORE_LIB_INCLUDES} 
		SYSTEM ${EXTERNAL_INCLUDES}
		)
endif()

#########################################################
## NetworkAPI version of hello

set(src_executable_napi_hello napi_hello)
add_executable(${src_executable_napi_hello} examples/napi_hello/napi_hello.cpp)
# link with the static library
target_link_libraries(${src_executable_napi_hello} 
    ${INTERNAL_LINKER_FLAGS}
    ${core_library}
    ${COMMON_OS_LIBS}
)                  
target_compile_options( ${src_executable_napi_hello} PUBLIC ${INTERNAL_CXX_FLAGS})
target_compile_definitions(${src_executable_napi_hello} PRIVATE ${COMMON_COMPILER_DEFINITIONS})
target_include_directories(${src_executable_napi_hello} PRIVATE 
		${CORE_LIB_INCLUDES} 
		SYSTEM ${EXTERNAL_INCLUDES}
		)

#########################################################
## NetworkAPI version of hello with database

set(src_executable_napi_hello_database napi_hello_database)
add_executable(${src_executable_napi_hello_database} examples/napi_hello/napi_hello_database.cpp)
# link with the static library
target_link_libraries(${src_executable_napi_hello_database} 
    ${INTERNAL_LINKER_FLAGS}
    ${core_library}
    ${COMMON_OS_LIBS}
)                  
target_compile_options( ${src_executable_napi_hello_database} PUBLIC ${INTERNAL_CXX_FLAGS})
target_compile_definitions(${src_executable_napi_hello_database} PRIVATE ${COMMON_COMPILER_DEFINITIONS})
target_include_directories(${src_executable_napi_hello_database} PRIVATE 
		${CORE_LIB_INCLUDES} 
		SYSTEM ${EXTERNAL_INCLUDES}
		)
		
#########################################################
## MNIST Spatial Pooler Example
#
set(src_executable_mnistsp mnist_sp)
add_executable(${src_executable_mnistsp} examples/mnist/MNIST_SP.cpp)
target_link_libraries(${src_executable_mnistsp} 
        ${INTERNAL_LINKER_FLAGS}
        ${core_library}
        ${COMMON_OS_LIBS}
)
  target_compile_options(${src_executable_mnistsp} PUBLIC ${INTERNAL_CXX_FLAGS})
  target_compile_definitions(${src_executable_mnistsp} PRIVATE ${COMMON_COMPILER_DEFINITIONS})
  # Pass MNIST data directory to main.cpp
  target_compile_definitions(${src_executable_mnistsp} PRIVATE MNIST_DATA_LOCATION=${mnist_SOURCE_DIR})
  target_include_directories(${src_executable_mnistsp} PRIVATE 
        ${CORE_LIB_INCLUDES} 
	SYSTEM ${EXTERNAL_INCLUDES}
        )

###########################################################
## REST server and client examples
#
  set(src_executable_rest_server rest_server)
  add_executable(${src_executable_rest_server} examples/rest/server.cpp)
  target_link_libraries(${src_executable_rest_server} 
          ${INTERNAL_LINKER_FLAGS}
          ${core_library}
          ${COMMON_OS_LIBS}
  )
  target_compile_options(${src_executable_rest_server} PUBLIC ${INTERNAL_CXX_FLAGS})
  target_compile_definitions(${src_executable_rest_server} PRIVATE ${COMMON_COMPILER_DEFINITIONS})
  target_include_directories(${src_executable_rest_server} PRIVATE 
        ${CORE_LIB_INCLUDES} 
        examples/rest
	SYSTEM ${EXTERNAL_INCLUDES}
        )
    
  set(src_executable_rest_client rest_client)
  add_executable(${src_executable_rest_client} examples/rest/client.cpp)
  target_link_libraries(${src_executable_rest_client} 
          ${INTERNAL_LINKER_FLAGS}
          ${core_library}
          ${COMMON_OS_LIBS}
  )
  target_compile_options(${src_executable_rest_client} PUBLIC ${INTERNAL_CXX_FLAGS})
  target_compile_definitions(${src_executable_rest_client} PRIVATE ${COMMON_COMPILER_DEFINITIONS})
  target_include_directories(${src_executable_rest_client} PRIVATE 
        ${CORE_LIB_INCLUDES} 
	SYSTEM ${EXTERNAL_INCLUDES}
        )

############ INSTALL ######################################
#
# Install targets into CMAKE_INSTALL_PREFIX
#
install(TARGETS
        ${core_library}
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
        
install(TARGETS
        ${src_executable_hello}
        ${src_executable_napi_hello}
        ${src_executable_napi_hello_database}
        ${src_executable_mnistsp}
        ${src_executable_rest_server}
        ${src_executable_rest_client}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
if(MSVC)
else()	
  install(TARGETS
        ${src_dyn_executable_hello}
        ${src_lib_shared}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
endif()

# Version.hpp is also used by the htm.bindings release/deployment system
install(FILES ${PROJECT_BINARY_DIR}/Version.hpp
      DESTINATION include)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/htm
      MESSAGE_NEVER
      DESTINATION include/
      FILES_MATCHING PATTERN "*.h*"
      PATTERN "*.hpp.in" EXCLUDE)

install(DIRECTORY ${cereal_INCLUDE_DIRS}/cereal
      MESSAGE_NEVER
      DESTINATION include/
      FILES_MATCHING PATTERN "*.h*"
      PATTERN "*.hpp.in" EXCLUDE)

install(FILES ${cpp-httplib_INCLUDE_DIRS}/httplib.h
      DESTINATION include)

install(DIRECTORY "${REPOSITORY_DIR}/external/common/include/"
      MESSAGE_NEVER
      DESTINATION include)
      
# include the python wheel if it exists.
# run "python3 setup.py install --user"
# and "python3 setup.py bdist_wheel"
# prior to this to create the whl file.
install(DIRECTORY "${CMAKE_INSTALL_PREFIX}/distr/dist/"
	  MESSAGE_NEVER
	  DESTINATION py
	  FILES_MATCHING PATTERN "*.whl")

# copy requirements.txt into dist/ 
install(FILES ${REPOSITORY_DIR}/requirements.txt
        DESTINATION distr/dist)


############### PACKAGE ###########################
#
# `make package` results in
# htm_cpp-${VERSION}-${PLATFORM}${BITNESS}${PLATFORM_SUFFIX}.tar.gz binary release
#
#   TODO:  The version numbers need to be automatically incremented.
message(STATUS "Packaging version: ${VERSION} in CPACK")
set(CPACK_PACKAGE_NAME "htm.core")
set(CPACK_PACKAGE_VENDOR "https://github.com/htm-community/htm.core")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "htm.core - Community implementation of Numenta's algorthms.")
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PATCH})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "htm.core")

if(MSVC)
	set(CPACK_GENERATOR "ZIP")
else()
	set(CPACK_GENERATOR "TGZ")
endif()
set(CPACK_RESOURCE_FILE_LICENSE "${REPOSITORY_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_FILE_NAME "htm_core-${VERSION}-${PLATFORM}${BITNESS}${PLATFORM_SUFFIX}")
include(CPack)
