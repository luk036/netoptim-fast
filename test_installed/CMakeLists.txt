cmake_minimum_required(VERSION 3.14)
project(NetOptimFastInstalledTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find dependencies first - the installed config references fmt, Py2Cpp, MyWheel and DiGraphXFast.
find_package(Threads REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(Py2Cpp REQUIRED)
find_package(MyWheel REQUIRED)
find_package(DiGraphXFast REQUIRED)
find_package(NetOptimFast REQUIRED)

add_executable(netoptimfast_installed_test main.cpp)
target_link_libraries(netoptimfast_installed_test PRIVATE NetOptimFast::NetOptimFast)

enable_testing()
add_test(NAME netoptimfast_installed_test COMMAND netoptimfast_installed_test)
