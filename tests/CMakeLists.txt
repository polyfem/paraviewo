################################################################################
# Tests
################################################################################

set(test_sources
	test.cpp
)

add_executable(paraviewo_unit_tests ${test_sources})

################################################################################
# Required Libraries
################################################################################

target_link_libraries(paraviewo_unit_tests PUBLIC paraviewo::paraviewo)

include(paraviewo_warnings)
target_link_libraries(paraviewo_unit_tests PUBLIC paraviewo::warnings)

include(catch2)
target_link_libraries(paraviewo_unit_tests PUBLIC Catch2::Catch2WithMain)

################################################################################
# Compiler options
################################################################################

target_compile_definitions(paraviewo_unit_tests PUBLIC -DPARAVIEWO_TEST_DIR=\"${CMAKE_SOURCE_DIR}/tests\")

target_compile_definitions(paraviewo_unit_tests PUBLIC CATCH_CONFIG_ENABLE_BENCHMARKING)

################################################################################
# Register tests
################################################################################

FetchContent_GetProperties(catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)

# Register tests
set(PARSE_CATCH_TESTS_ADD_TO_CONFIGURE_DEPENDS ON)
catch_discover_tests(paraviewo_unit_tests)