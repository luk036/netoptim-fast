cmake_minimum_required(VERSION 3.14...3.22)

project(NetOptimFastTests LANGUAGES CXX)

# ---- Options ----

option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)
option(TEST_INSTALLED_VERSION "Test the version found by find_package" OFF)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)
include(../cmake/specific.cmake)

CPMAddPackage("gh:doctest/doctest@2.5.2")
CPMAddPackage("gh:TheLartians/Format.cmake@1.7.3")

# Add the library target if it doesn't already exist (e.g. when built from the root project via
# add_subdirectory). When built standalone with -Stest or via all/, it needs to be added here.
if(TEST_INSTALLED_VERSION)
  find_package(NetOptimFast REQUIRED)
elseif(NOT TARGET NetOptimFast)
  CPMAddPackage(NAME NetOptimFast SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
endif()

# ---- Create binary ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} doctest::doctest NetOptimFast::NetOptimFast ${SPECIFIC_LIBS})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)

# enable compiler warnings for the test target
if(NOT TEST_INSTALLED_VERSION)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wpedantic -Wextra -Werror)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wpedantic -Wextra -Werror)
  elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8 /W4 /WX)
  endif()
endif()

# ---- Add tests ----

enable_testing()

# Add a single test that runs all test cases
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

# ---- code coverage ----

if(ENABLE_TEST_COVERAGE)
  target_compile_options(NetOptimFast INTERFACE -O0 -g -fprofile-arcs -ftest-coverage)
  target_link_options(NetOptimFast INTERFACE -fprofile-arcs -ftest-coverage)
endif()
