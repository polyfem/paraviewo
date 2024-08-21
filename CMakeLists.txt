# Detects whether this is a top-level project
get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT)
    set(PARAVIEWO_TOPLEVEL_PROJECT OFF)
else()
    set(PARAVIEWO_TOPLEVEL_PROJECT ON)
endif()

# Check required CMake version
set(REQUIRED_CMAKE_VERSION "3.14.0")
if(PARAVIEWO_TOPLEVEL_PROJECT)
    cmake_minimum_required(VERSION ${REQUIRED_CMAKE_VERSION})
else()
    # Don't use cmake_minimum_required here to avoid implicitly overriding parent policies
    if(${CMAKE_VERSION} VERSION_LESS ${REQUIRED_CMAKE_VERSION})
        message(FATAL_ERROR "CMake required version to build ParaviewO is ${REQUIRED_CMAKE_VERSION}")
    endif()
endif()

# Include user-provided default options if available. We do that before the main
# `project()` so that we can define the C/C++ compilers from the option file.
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ParaviwoOptions.cmake)
    message(STATUS "Using local options file: ${CMAKE_CURRENT_SOURCE_DIR}/ParaviwoOptions.cmake")
    include(${CMAKE_CURRENT_SOURCE_DIR}/ParaviwoOptions.cmake)
endif()

# Enable ccache if available
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  option(PARAVIWO_WITH_CCACHE "Enable ccache when building ParaviewO" ${PARAVIEWO_TOPLEVEL_PROJECT})
else()
  option(PARAVIWO_WITH_CCACHE "Enable ccache when building ParaviewO" OFF)
endif()
if(PARAVIWO_WITH_CCACHE AND CCACHE_PROGRAM)
  message(STATUS "Enabling Ccache support")
  set(ccacheEnv
    CCACHE_BASEDIR=${CMAKE_BINARY_DIR}
    CCACHE_SLOPPINESS=clang_index_store,include_file_ctime,include_file_mtime,locale,pch_defines,time_macros
  )
  foreach(lang IN ITEMS C CXX)
    set(CMAKE_${lang}_COMPILER_LAUNCHER
      ${CMAKE_COMMAND} -E env ${ccacheEnv} ${CCACHE_PROGRAM}
    )
  endforeach()
endif()

################################################################################
# CMake Policies
################################################################################

cmake_policy(SET CMP0054 NEW) # Only interpret if() arguments as variables or keywords when unquoted.
cmake_policy(SET CMP0076 NEW) # target_sources() command converts relative paths to absolute.
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24")
  cmake_policy(SET CMP0135 NEW) # Set the timestamps of all extracted contents to the time of the extraction.
endif()

################################################################################

project(ParaviewO DESCRIPTION "A simple C++ library to write Paraview Files" LANGUAGES C CXX)

option(PARAVIEWO_WITH_TESTS       "Enables unit test"                  ON)
option(PARAVIEWO_BUILD_DOCS       "Build documentation using Doxygen" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/paraviewo/")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/recipes/")

# Set default minimum C++ standard
if(PARAVIEWO_TOPLEVEL_PROJECT)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

### Configuration
set(PARAVIEWO_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src/paraviewo")
set(PARAVIEWO_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/src")

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/paraviewo/")
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/recipes/")

# General CMake utils
include(paraviewo_cpm_cache)
include(paraviewo_use_colors)

# Generate position independent code by default
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

################################################################################

# Sanitizers
include(sanitizers)

################################################################################
# ParaviewO library
################################################################################

add_library(paraviewo)
add_library(paraviewo::paraviewo ALIAS paraviewo)
# set_property(TARGET paraviewo PROPERTY COMPILE_WARNING_AS_ERROR ON)

# Add source and header files to paraviewo
add_subdirectory(${PARAVIEWO_SOURCE_DIR})

# Public include directory for paraviewo
target_include_directories(paraviewo PUBLIC ${PARAVIEWO_INCLUDE_DIR})

# Use C++17
target_compile_features(paraviewo PUBLIC cxx_std_17)

if (MSVC)
    add_compile_options(/bigobj)
endif ()

################################################################################
# Required libraries
################################################################################

# Eigen
include(eigen)
target_link_libraries(paraviewo PUBLIC Eigen3::Eigen)

# TinyXML library
include(tinyxml)
target_link_libraries(paraviewo PUBLIC tinyxml2)

# h5pp + HDF5 library
include(hdf5)
target_link_libraries(paraviewo PUBLIC hdf5::hdf5)

include(h5pp)
target_link_libraries(paraviewo PUBLIC h5pp::h5pp)

# Extra warnings (link this here so it has top priority)
include(paraviewo_warnings)
target_link_libraries(paraviewo PRIVATE paraviewo::warnings)

# Documentation
if(PARAVIEWO_TOPLEVEL_PROJECT AND PARAVIEWO_BUILD_DOCS)
    # check if Doxygen is installed
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        add_custom_target(paraviewo_doc_doxygen ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM)
    else (DOXYGEN_FOUND)
        message("Doxygen need to be installed to generate the doxygen documentation")
    endif (DOXYGEN_FOUND)
endif()

# Compile extras only if this is a top-level project
if(PARAVIEWO_TOPLEVEL_PROJECT AND PARAVIEWO_WITH_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()
