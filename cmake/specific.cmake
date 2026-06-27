set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

CPMAddPackage(
  NAME fmt
  GIT_TAG 12.1.0
  GITHUB_REPOSITORY fmtlib/fmt
  OPTIONS "FMT_INSTALL YES" # create an installable target
)

CPMAddPackage(
  NAME Py2Cpp
  GIT_TAG 1.6.0
  GITHUB_REPOSITORY luk036/py2cpp
  OPTIONS "INSTALL_ONLY YES" # create an installable target
)

CPMAddPackage(
  NAME MyWheel
  GIT_TAG 1.1.3
  GITHUB_REPOSITORY luk036/mywheel-cpp
  OPTIONS "INSTALL_ONLY YES" # create an installable target
)

# spdlog must be added before DiGraphXFast to ensure SPDLOG_FMT_EXTERNAL is set, avoiding linker
# conflicts with fmt when local fmt package is a shared library
CPMAddPackage(
  NAME spdlog
  GIT_TAG v1.17.0
  GITHUB_REPOSITORY gabime/spdlog
  OPTIONS "SPDLOG_INSTALL YES" "SPDLOG_FMT_EXTERNAL YES"
)

CPMAddPackage(
  NAME DiGraphXFast
  GIT_TAG v1.1.5
  GITHUB_REPOSITORY luk036/digraphx-fast
)

set(SPECIFIC_LIBS Threads::Threads fmt::fmt MyWheel::MyWheel Py2Cpp::Py2Cpp spdlog::spdlog
                  DiGraphXFast::DiGraphXFast
)
