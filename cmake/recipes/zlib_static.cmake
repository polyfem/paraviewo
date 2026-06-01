# zlib (https://github.com/madler/zlib)
# License: zlib

if(TARGET ZLIB::ZLIBSTATIC)
    return()
endif()

message(STATUS "Third-party: creating target 'ZLIB::ZLIBSTATIC'")

set(ZLIB_BUILD_TESTING OFF)

include(CPM)
CPMAddPackage("gh:madler/zlib@1.3.2")
