cmake_minimum_required(VERSION 3.14...3.22)

project(
  NetOptimFast
  VERSION 1.0.2
  LANGUAGES CXX
)

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

# ---------------------------------------------------------------------------
# Compiler warnings (as errors)
# ---------------------------------------------------------------------------

function(target_enable_warnings TARGET)
  if(MSVC)
    target_compile_options(${TARGET} PRIVATE /W4 /WX /utf-8 /wd4819)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(${TARGET} PRIVATE -Wall -Wextra -Wpedantic -Werror)
  endif()
endfunction()

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(NETOPTIMFAST_BUILD_TESTS "Build tests" ON)
option(NETOPTIMFAST_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(NETOPTIMFAST_BUILD_DOCS "Build Doxygen documentation" OFF)
option(NETOPTIMFAST_ENABLE_CLANG_TIDY "Enable clang-tidy analysis" OFF)
option(NETOPTIMFAST_ENABLE_COVERAGE "Enable coverage reporting" OFF)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

# Download CPM.cmake for reproducible dependency management see
# https://github.com/cpm-cmake/CPM.cmake for more info
set(CPM_DOWNLOAD_VERSION 0.38.5)
set(CPM_HASH_SUM "192aa0ccdc57dfe75bd9e4b176bf7fb5692fd2b3e3f7b09c74856fc39572b31c")
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
  message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
  file(DOWNLOAD
       https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
       ${CPM_DOWNLOAD_LOCATION} EXPECTED_HASH SHA256=${CPM_HASH_SUM}
  )
endif()
include(${CPM_DOWNLOAD_LOCATION})

# PackageProject.cmake will be used to make our target installable
CPMAddPackage("gh:TheLartians/PackageProject.cmake@1.8.0")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# fmt is a transitive dependency of the graph libraries; prefer a system-installed fmt when
# available.
find_package(fmt CONFIG QUIET)
if(fmt_FOUND)
  message(STATUS "Found system fmt: ${fmt_DIR}")
  set(CPM_fmt_ADDED YES)
else()
  CPMAddPackage(
    NAME fmt
    GIT_TAG 12.1.0
    GITHUB_REPOSITORY fmtlib/fmt
    OPTIONS "FMT_INSTALL YES" # create an installable target
  )
endif()

CPMAddPackage(
  NAME Py2Cpp
  GIT_TAG v1.6.4
  GITHUB_REPOSITORY luk036/py2cpp
  OPTIONS "INSTALL_ONLY YES" "PY2CPP_BUILD_TESTS OFF"
)

CPMAddPackage(
  NAME MyWheel
  GIT_TAG v1.1.6
  GITHUB_REPOSITORY luk036/mywheel-cpp
  OPTIONS "INSTALL_ONLY YES" "MYWHEEL_BUILD_TESTS OFF"
)

# spdlog must be added before DiGraphXFast to ensure SPDLOG_FMT_EXTERNAL is set, avoiding linker
# conflicts with fmt when local fmt package is a shared library Try system-installed spdlog first
# (Ubuntu: libspdlog-dev, macOS: brew install spdlog, Termux: spdlog)
find_package(spdlog CONFIG QUIET)
if(spdlog_FOUND)
  message(STATUS "Found system spdlog: ${spdlog_DIR}")
  set(CPM_spdlog_ADDED YES)
  # Some system spdlog packages (e.g. conda) only export the namespaced spdlog::spdlog target, while
  # DiGraphXFast queries the bare `spdlog` target for its interface definitions. Provide an alias so
  # both work whichever spdlog is used.
  if(NOT TARGET spdlog AND TARGET spdlog::spdlog)
    add_library(spdlog ALIAS spdlog::spdlog)
  endif()
else()
  CPMAddPackage(
    NAME spdlog
    GIT_TAG v1.17.0
    GITHUB_REPOSITORY gabime/spdlog
    OPTIONS "SPDLOG_INSTALL YES" "SPDLOG_FMT_EXTERNAL YES"
  )
endif()

CPMAddPackage(
  NAME DiGraphXFast
  GIT_TAG v1.1.7
  GITHUB_REPOSITORY luk036/digraphx-fast
  OPTIONS "INSTALL_ONLY YES" "DIGRAPHXFAST_BUILD_TESTS OFF"
)

set(SPECIFIC_LIBS Threads::Threads fmt::fmt MyWheel::MyWheel Py2Cpp::Py2Cpp spdlog::spdlog
                  DiGraphXFast::DiGraphXFast
)

# ---------------------------------------------------------------------------
# Library target (header-only)
# ---------------------------------------------------------------------------

# Note: globbing sources is considered bad practice as CMake's generators may not detect new files
# automatically. Keep that in mind when changing files, or explicitly mention them here.
file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
     "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

add_library(${PROJECT_NAME} INTERFACE)

# being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(${PROJECT_NAME} INTERFACE "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")

target_include_directories(
  ${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                            $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

# ---- Create an installable target ----
# this allows users to install and find the library via `find_package()`.

# the location where the project's version header will be placed should match the project's regular
# header paths
set(VERSION_HEADER_LOCATION "netoptim_fast/version.h")

packageProject(
  NAME ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  NAMESPACE ${PROJECT_NAME}
  BINARY_DIR ${PROJECT_BINARY_DIR}
  INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
  INCLUDE_DESTINATION include/${PROJECT_NAME}-${PROJECT_VERSION}
  VERSION_HEADER "${VERSION_HEADER_LOCATION}"
  COMPATIBILITY SameMajorVersion
  DEPENDENCIES "fmt 12.1.0" "Py2Cpp 1.6.4" "MyWheel 1.1.6"
)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

enable_testing()

if(NETOPTIMFAST_BUILD_TESTS)
  # Try system-installed doctest first (Ubuntu: doctest-dev, macOS: brew install doctest, Termux:
  # doctest)
  find_package(doctest CONFIG QUIET)

  if(NOT doctest_FOUND)
    CPMAddPackage(
      NAME doctest
      GITHUB_REPOSITORY doctest/doctest
      VERSION 2.5.2
      OPTIONS "DOCTEST_NO_INSTALL ON" # keep the installed prefix minimal
    )
  endif()

  # Code formatting targets (format / fix-format) via Format.cmake
  CPMAddPackage("gh:TheLartians/Format.cmake@1.7.3")

  file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "test/source/*.cpp")
  add_executable(NetOptimFastTests ${TEST_SOURCES})
  target_link_libraries(
    NetOptimFastTests PRIVATE doctest::doctest NetOptimFast::NetOptimFast ${SPECIFIC_LIBS}
  )
  set_target_properties(NetOptimFastTests PROPERTIES CXX_STANDARD 20)
  target_enable_warnings(NetOptimFastTests)
  add_test(NAME NetOptimFastTests COMMAND NetOptimFastTests)
endif()

# ---------------------------------------------------------------------------
# Benchmarks
# ---------------------------------------------------------------------------

if(NETOPTIMFAST_BUILD_BENCHMARKS)
  # nanobench is a header-only benchmarking library
  set(NANOBENCH_VERSION 4.3.11)
  set(NANOBENCH_DIR "${CMAKE_BINARY_DIR}/_deps/nanobench")
  set(NANOBENCH_HEADER "${NANOBENCH_DIR}/nanobench.h")

  if(NOT EXISTS "${NANOBENCH_HEADER}")
    file(MAKE_DIRECTORY "${NANOBENCH_DIR}")
    file(
      DOWNLOAD
      "https://raw.githubusercontent.com/martinus/nanobench/v${NANOBENCH_VERSION}/src/include/nanobench.h"
      "${NANOBENCH_HEADER}"
      STATUS download_status
    )
    list(GET download_status 0 status_code)
    if(NOT status_code EQUAL 0)
      message(FATAL_ERROR "Failed to download nanobench (status ${status_code})")
    endif()
  endif()

  add_library(nanobench INTERFACE)
  target_include_directories(nanobench SYSTEM INTERFACE "${NANOBENCH_DIR}")

  file(GLOB_RECURSE ALL_BENCH_CPP CONFIGURE_DEPENDS "bench/*.cpp")
  foreach(ONE_BENCH_CPP ${ALL_BENCH_CPP})
    get_filename_component(ONE_BENCH_EXEC ${ONE_BENCH_CPP} NAME_WE)
    set(TARGET_NAME Bench_${ONE_BENCH_EXEC})

    add_executable(${TARGET_NAME} ${ONE_BENCH_CPP})
    set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME ${ONE_BENCH_EXEC} CXX_STANDARD 20)
    target_link_libraries(
      ${TARGET_NAME} PRIVATE nanobench NetOptimFast::NetOptimFast ${SPECIFIC_LIBS}
    )
    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_enable_warnings(${TARGET_NAME})
    add_test(NAME ${ONE_BENCH_EXEC} COMMAND ${TARGET_NAME})
  endforeach()
endif()

# ---------------------------------------------------------------------------
# clang-tidy
# ---------------------------------------------------------------------------

if(NETOPTIMFAST_ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY clang-tidy)
  if(CLANG_TIDY)
    add_custom_target(
      clang-tidy
      COMMAND ${CLANG_TIDY} --extra-arg=-std=c++20 --extra-arg=-I${CMAKE_CURRENT_SOURCE_DIR}/include
              ${CMAKE_CURRENT_SOURCE_DIR}/include/netoptim_fast/*.hpp
      COMMENT "Running clang-tidy static analysis"
    )
  else()
    message(WARNING "clang-tidy not found - analysis target unavailable")
  endif()
endif()

# ---------------------------------------------------------------------------
# Doxygen documentation
# ---------------------------------------------------------------------------

if(NETOPTIMFAST_BUILD_DOCS)
  find_package(Doxygen QUIET)
  if(DOXYGEN_FOUND)
    set(DOXYGEN_PROJECT_NAME ${PROJECT_NAME})
    set(DOXYGEN_PROJECT_VERSION ${PROJECT_VERSION})
    set(DOXYGEN_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
    set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doxygen")

    configure_file(documentation/Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    add_custom_target(
      GenerateDocs
      COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
      COMMAND echo "Docs written to: ${DOXYGEN_OUTPUT_DIRECTORY}/html"
      COMMENT "Generating Doxygen HTML documentation in ${DOXYGEN_OUTPUT_DIRECTORY}/html"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
  else()
    add_custom_target(
      GenerateDocs COMMAND ${CMAKE_COMMAND} -E echo
                           "Doxygen not found - skipping documentation generation"
    )
  endif()
endif()

# ---------------------------------------------------------------------------
# Coverage report target (GCC/Clang only - requires gcovr)
# ---------------------------------------------------------------------------

if(NETOPTIMFAST_ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  if(TARGET NetOptimFastTests)
    target_compile_options(NetOptimFastTests PRIVATE -O0 -g -fprofile-arcs -ftest-coverage)
    target_link_options(NetOptimFastTests PRIVATE -fprofile-arcs -ftest-coverage)
  endif()

  find_program(GCOVR_PATH gcovr)
  if(GCOVR_PATH)
    set(extra_gcovr_args "")
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
      find_program(LLVM_COV_PATH llvm-cov)
      if(LLVM_COV_PATH)
        list(APPEND extra_gcovr_args --llvm-cov-binary ${LLVM_COV_PATH})
      endif()
    endif()
    add_custom_target(
      coverage
      COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
      COMMAND
        ${GCOVR_PATH} --root ${PROJECT_SOURCE_DIR} --html-details
        ${CMAKE_BINARY_DIR}/coverage/index.html --exclude test/ --exclude build/ ${extra_gcovr_args}
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMENT "Generating coverage report in ${CMAKE_BINARY_DIR}/coverage/index.html"
    )
  else()
    message(
      WARNING "gcovr not found - coverage target unavailable. Install with: pip install gcovr"
    )
  endif()
endif()
