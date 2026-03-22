# Development

This document describes the process of doing development for `libgeojson`.

# Prerequisties

`libgeojson` is built using CMake and thus requires a CMake install. This can
be done however you like.

In addition, it is recommended, but not required to get the project's
dependencies via [Conan](https://conan.io). This can be installed via `pip` or
some package managers.

If not using Conan, you should also install the other code dependencies, see
`cmake/Dependencies.cmake` for more details.

# Build Steps (Release)

1. In the repo root, run:

```bash
conan install . --build=missing
```

2. Run CMake configuration via:

```bash
cmake --preset conan-release
```

3. Build the project:

```bash
cmake --build --preset conan-release
```

4. Optional: Install the project:

```bash
cmake --install build/Release [--prefix <prefix>]
```

## Build Steps (Debug)

Same as above with some minore differences:

```bash
conan install . --build=missing -s build_type=Debug
cmake --preset conan-debug
cmake --build --preset conan-debug
cmake --install build/Debug [--prefix <prefix>]
```

## Running the Tests

Tests are all managed via CTest:

```bash
ctest --preset conan-release --output-on-failure
```

