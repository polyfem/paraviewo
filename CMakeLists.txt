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


################################################################################
# CMake Policies
################################################################################

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24")
    cmake_policy(SET CMP0135 NEW) # https://cmake.org/cmake/help/latest/policy/CMP0135.html
endif()

################################################################################
project(ParaviewO DESCRIPTION "A simple C++ library to write Paraview Files" LANGUAGES C CXX)
################################################################################

option(PARAVIEWO_BUILD_DOCS       "Build documentation using Doxygen" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/recipes/")

# Color output
include(UseColors)

# Prepend function
include(PrependCurrentPath)

# Extra warnings
include(POWarnings)

# Use C++11/14
include(CXXFeatures)

# Sort projects inside the solution
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

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


# Public include directory for paraviewo
target_include_directories(paraviewo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Use C++17
target_compile_features(paraviewo PUBLIC ${CXX17_FEATURES})

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

# HighFive + HDF5 library
include(hdf5)
target_link_libraries(paraviewo PUBLIC hdf5::hdf5)

include(high_five)
target_link_libraries(paraviewo PUBLIC HighFive::HighFive)

# Extra warnings (link this here so it has top priority)
target_link_libraries(paraviewo PRIVATE paraviewo::warnings)

# Documentation
if(PARAVIEWO_TOPLEVEL_PROJECT)

    if(PARAVIEWO_BUILD_DOCS)
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
endif()

# Other sources
add_subdirectory(src/paraviewo)

# Compile extras only if this is a top-level project
# if(PARAVIEWO_TOPLEVEL_PROJECT)
#     # Unit tests
#     if(PARAVIEWO_WITH_TESTS)
#         include(CTest)
#         enable_testing()
#         add_subdirectory(tests)
#     endif()
# endif()
