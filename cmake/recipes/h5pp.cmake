# h5pp (https://github.com/wildmeshing/h5pp)
# License: mit

if(TARGET h5pp::h5pp)
    return()
endif()

message(STATUS "Third-party: creating target 'h5pp'")

SET(H5PP_ENABLE_EIGEN3 ON)
SET(H5PP_ENABLE_FMT ON)
SET(H5PP_IS_SUBPROJECT ON)

include(CPM)
CPMAddPackage("gh:wildmeshing/h5pp#9872c2f239271c711d572035cc689ae485e12a4c")