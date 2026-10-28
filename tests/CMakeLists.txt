# Tests
# doctest is vendored under third_party/doctest (header-only)
set(DOCTEST_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/doctest")

add_subdirectory(unit)
add_subdirectory(api)
add_subdirectory(integration)