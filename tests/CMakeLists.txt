################################################################################
# Tests
################################################################################

set(test_sources
	main.cpp
	test.cpp
)

add_executable(paraviewo_unit_tests ${test_sources})

################################################################################
# Required Libraries
################################################################################

target_link_libraries(paraviewo_unit_tests PUBLIC paraviewo::paraviewo)

target_link_libraries(paraviewo_unit_tests PUBLIC paraviewo::warnings)

include(catch2)
target_link_libraries(paraviewo_unit_tests PUBLIC Catch2::Catch2)

################################################################################
# Compiler options
################################################################################

target_compile_definitions(paraviewo_unit_tests PUBLIC -DPARAVIEWO_TEST_DIR=\"${CMAKE_SOURCE_DIR}/tests\")

target_compile_definitions(paraviewo_unit_tests PUBLIC CATCH_CONFIG_ENABLE_BENCHMARKING)

################################################################################
# Register tests
################################################################################

foreach(source IN ITEMS ${test_sources})
    source_group("tests" FILES "${source}")
endforeach()

set(PARSE_CATCH_TESTS_ADD_TO_CONFIGURE_DEPENDS ON)

# Include Catch2 and provide function `catch_discover_tests` to register tests.
FetchContent_GetProperties(catch2)
include("${catch2_SOURCE_DIR}/contrib/Catch.cmake")

catch_discover_tests(paraviewo_unit_tests)

