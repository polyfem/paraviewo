# hdf5 (https://github.com/HDFGroup/hdf5)
# License: ???

if(TARGET hdf5::hdf5)
    return()
endif()

# 1. Try finding a system-installed HDF5 first
find_package(HDF5 COMPONENTS C HL QUIET)

if(HDF5_FOUND)
    message(STATUS "Found system HDF5, skipping CPM download.")
    # Map the system target to the target expected by paraviewo
    if(NOT TARGET hdf5::hdf5)
        add_library(hdf5::hdf5 ALIAS hdf5::hdf5_cpp) # or hdf5::hdf5 depending on CMake version
    endif()
    return()
endif()

# 2. Fallback to CPM build
message(STATUS "Third-party: creating target 'hdf5'")

option(HDF5_GENERATE_HEADERS "" OFF)
option(HDF5_BUILD_EXAMPLES "" OFF)
option(HDF5_BUILD_TOOLS "" OFF)
option(HDF5_BUILD_UTILS "" OFF)
option(HDF5_BUILD_HL_TOOLS "" OFF)
option(HDF5_BUILD_HL_LIB "" ON)
option(HDF5_TEST_CPP "" OFF)
option(HDF5_TEST_EXAMPLES "" OFF)
option(HDF5_TEST_FORTRAN "" OFF)
option(HDF5_TEST_JAVA "" OFF)
option(HDF5_TEST_PARALLEL "" OFF)
option(HDF5_TEST_SERIAL "" OFF)
option(HDF5_TEST_SWMR "" OFF)
option(HDF5_TEST_TOOLS "" OFF)
option(HDF5_TEST_VFD "" OFF)
option(HDF5_ENABLE_ALL_WARNINGS "" OFF)
option(HDF5_ENABLE_EMBEDDED_LIBINFO "" OFF)

# Tell hdf5 to use our zlib. See config/HDF5UseZLIB.cmake in hdf5 for exact logic.
include(zlib_static)
set(H5_ZLIB_HEADER "zlib.h")
set(ZLIB_FOUND TRUE)

#To prevent changes in the oput dirs
set (HDF5_EXTERNALLY_CONFIGURED 1)

include(CPM)
set(HDF5_RELEASE_TAG hdf5-1_14_3)

#we fetch the zip file to get prebuilt files (and avoid a perl dependency)
CPMAddPackage("https://github.com/HDFGroup/hdf5/releases/download/${HDF5_RELEASE_TAG}/${HDF5_RELEASE_TAG}.zip")

# This cmake policy allows us to link target defined in other directory.
# For unknown reason, this policy is set to OLD in polyFEM CI.
if(POLICY CMP0079)
      cmake_policy(SET CMP0079 NEW)
endif()
target_link_libraries(hdf5-static PUBLIC ZLIB::ZLIBSTATIC)

target_link_libraries(hdf5-static INTERFACE hdf5_hl-static)
add_library(hdf5::hdf5 ALIAS hdf5-static)
