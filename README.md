
[![Actions Status](https://github.com/luk036/netoptim-fast/workflows/MacOS/badge.svg)](https://github.com/luk036/netoptim-fast/actions)
[![Actions Status](https://github.com/luk036/netoptim-fast/workflows/Windows/badge.svg)](https://github.com/luk036/netoptim-fast/actions)
[![Actions Status](https://github.com/luk036/netoptim-fast/workflows/Ubuntu/badge.svg)](https://github.com/luk036/netoptim-fast/actions)
[![Actions Status](https://github.com/luk036/netoptim-fast/workflows/Install/badge.svg)](https://github.com/luk036/netoptim-fast/actions)
[![codecov](https://codecov.io/gh/luk036/netoptim-fast/graph/badge.svg)](https://codecov.io/gh/luk036/netoptim-fast)

# ⚡ netoptim-fast

Fast specialized separation oracles for network optimization.

## ✨ Features

- [Modern CMake practices](https://pabloariasal.github.io/2018/02/19/its-time-to-do-cmake-right/)
- Header-only library
- Integrated test suite
- Continuous integration via [GitHub Actions](https://help.github.com/en/actions/)
- Code coverage via [codecov](https://codecov.io)
- Code formatting enforced by [clang-format](https://clang.llvm.org/docs/ClangFormat.html) and [cmake-format](https://github.com/cheshirekow/cmake_format) via [Format.cmake](https://github.com/TheLartians/Format.cmake)
- Reproducible dependency management via [CPM.cmake](https://github.com/TheLartians/CPM.cmake)
- Installable target with automatic versioning information and header generation via [PackageProject.cmake](https://github.com/TheLartians/PackageProject.cmake)

## Usage

### Build and run test suite

Use the following commands from the project's root directory to run the test suite.

```bash
cmake -S. -Bbuild
cmake --build build
cd build/test
CTEST_OUTPUT_ON_FAILURE=1 ctest
```

To collect code coverage information, run CMake with the `-DENABLE_TEST_COVERAGE=1` option.

### Run clang-format

Use the following commands from the project's root directory to check and fix C++ and CMake source style.
This requires _clang-format_, _cmake-format_ and _pyyaml_ to be installed on the current system.

```bash
cmake -S. -Bbuild

# view changes
cmake --build build --target format

# apply changes
cmake --build build --target fix-format
```

See [Format.cmake](https://github.com/TheLartians/Format.cmake) for details.

### Additional tools

The test subproject includes the [tools.cmake](cmake/tools.cmake) file which is used to import additional tools on-demand through CMake configuration arguments.
The following are currently supported.

#### Sanitizers

Sanitizers can be enabled by configuring CMake with `-DUSE_SANITIZER=<Address | Memory | MemoryWithOrigins | Undefined | Thread | Leak | 'Address;Undefined'>`.

#### Static Analyzers

Static Analyzers can be enabled by setting `-DUSE_STATIC_ANALYZER=<clang-tidy | iwyu | cppcheck>`, or a combination of those in quotation marks, separated by semicolons.
By default, analyzers will automatically find configuration files such as `.clang-format`.
Additional arguments can be passed to the analyzers by setting the `CLANG_TIDY_ARGS`, `IWYU_ARGS` or `CPPCHECK_ARGS` variables.

#### Ccache

Ccache can be enabled by configuring with `-DUSE_CCACHE=<ON | OFF>`.
