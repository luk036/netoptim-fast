
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
cmake -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

To collect code coverage information, run CMake with the `-DNETOPTIMFAST_ENABLE_COVERAGE=1` option.

### Run clang-format

Use the following commands from the project's root directory to check and fix C++ and CMake source style.
This requires _clang-format_, _cmake-format_ and _pyyaml_ to be installed on the current system.

```bash
cmake -B build

# view changes
cmake --build build --target format

# apply changes
cmake --build build --target fix-format
```

See [Format.cmake](https://github.com/TheLartians/Format.cmake) for details.

### Build the documentation

```bash
cmake -B build -DNETOPTIMFAST_BUILD_DOCS=ON
cmake --build build --target GenerateDocs
# view the docs
open build/doxygen/html/index.html
```

To build the documentation locally, you will need Doxygen and Graphviz installed on your system.

### Additional tools

The test subproject includes the [tools.cmake](cmake/tools.cmake) file which is used to import additional tools on-demand through CMake configuration arguments.
The following are currently supported.

#### Sanitizers

Sanitizers can be enabled by configuring CMake with `-DUSE_SANITIZER=<Address | Memory | MemoryWithOrigins | Undefined | Thread | Leak | 'Address;Undefined'>`.

#### Static Analyzers

clang-tidy can be enabled by configuring CMake with `-DNETOPTIMFAST_ENABLE_CLANG_TIDY=ON` and building the `clang-tidy` target.
It analyzes the public headers using the checks configured in `.clang-tidy`.

```bash
cmake -B build -DNETOPTIMFAST_ENABLE_CLANG_TIDY=ON
cmake --build build --target clang-tidy
```

#### Ccache

Ccache can be enabled by configuring with `-DUSE_CCACHE=<ON | OFF>`.
