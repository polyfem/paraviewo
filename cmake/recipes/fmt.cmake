# fmt
# License: mit

if(TARGET fmt::fmt)
    return()
endif()

message(STATUS "Third-party: creating target 'fmt::fmt'")


include(FetchContent)
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 9.1.0
    GIT_SHALLOW FALSE
)
FetchContent_MakeAvailable(fmt)
