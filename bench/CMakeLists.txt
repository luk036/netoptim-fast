cmake_minimum_required(VERSION 3.14...3.22)

project(NetOptimFastBenchmarks LANGUAGES CXX)

# ---- Options ----

option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)

# ---- Dependencies ----

include(../cmake/CPM.cmake)
include(../cmake/tools.cmake)
include(../cmake/specific.cmake)

# Add the library target if it doesn't exist (standalone build)
if(NOT TARGET NetOptimFast)
  CPMAddPackage(NAME NetOptimFast SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
endif()

# ---- BM_network_oracle ----

add_executable(BM_network_oracle BM_network_oracle.cpp)
target_link_libraries(BM_network_oracle NetOptimFast::NetOptimFast ${SPECIFIC_LIBS})
target_include_directories(BM_network_oracle PRIVATE ../include)
set_target_properties(BM_network_oracle PROPERTIES CXX_STANDARD 20)

# enable compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(BM_network_oracle PRIVATE -Wall -Wpedantic -Wextra -Werror)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_compile_options(BM_network_oracle PRIVATE -Wall -Wpedantic -Wextra -Werror)
elseif(MSVC)
  target_compile_options(BM_network_oracle PRIVATE /utf-8 /W4 /WX)
endif()
