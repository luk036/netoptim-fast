cmake_minimum_required(VERSION 3.14...3.22)

project(NetOptimFastTests LANGUAGES CXX)

# ---- Options ----

option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)
option(TEST_INSTALLED_VERSION "Test the version found by find_package" OFF)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

if(TEST_INSTALLED_VERSION)
  # All dependencies are installed — use find_package to avoid CPM re-fetching fmt/spdlog
  # transitively, causing ALIAS conflicts
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
  find_package(fmt REQUIRED)
  find_package(spdlog REQUIRED)
  find_package(Py2Cpp REQUIRED)
  find_package(MyWheel REQUIRED)
  find_package(DiGraphXFast REQUIRED)
  set(SPECIFIC_LIBS Threads::Threads fmt::fmt MyWheel::MyWheel Py2Cpp::Py2Cpp spdlog::spdlog
                    DiGraphXFast::DiGraphXFast
  )
  find_package(NetOptimFast REQUIRED)
else()
  include(../cmake/specific.cmake)

  if(NOT TARGET NetOptimFast)
    CPMAddPackage(NAME NetOptimFast SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
  endif()
endif()

# Try system-installed doctest first (Ubuntu: doctest-dev, macOS: brew install doctest, Termux:
# doctest)
find_package(doctest CONFIG QUIET)

if(NOT doctest_FOUND)
  CPMAddPackage("gh:doctest/doctest@2.5.2")
endif()
CPMAddPackage("gh:TheLartians/Format.cmake@1.7.3")

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
