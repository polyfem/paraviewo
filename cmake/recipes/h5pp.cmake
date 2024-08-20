# h5pp (https://github.com/wildmeshing/h5pp)
# License: mit

if(TARGET h5pp::h5pp)
    return()
endif()

message(STATUS "Third-party: creating target 'h5pp'")

SET(H5PP_ENABLE_EIGEN3 ON)
SET(H5PP_ENABLE_FMT OFF)
SET(H5PP_ENABLE_SPDLOG OFF)
SET(H5PP_IS_SUBPROJECT ON)

include(CPM)
CPMAddPackage("gh:wildmeshing/h5pp#b46d66b16c11f4e94752433e61d92b152be3d1d8")
