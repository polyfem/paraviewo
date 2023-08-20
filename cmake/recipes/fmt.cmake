# fmt (https://github.com/fmtlib/fmt)
# License: mit

if(TARGET fmt::fmt)
    return()
endif()

message(STATUS "Third-party: creating target 'fmt::fmt'")

include(CPM)
CPMAddPackage("gh:fmtlib/fmt#10.1.0")