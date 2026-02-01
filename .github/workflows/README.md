# GitHub CI Configuration for libgeojson

This directory contains GitHub Actions workflows for continuous integration of the libgeojson project.

## Workflows

### 1. CI Workflow (`ci.yml`)

This is the main continuous integration workflow that builds and tests the project across multiple platforms and compilers.

#### Matrix Builds

**Linux (Ubuntu 22.04 & 24.04):**
- g++ (Debug & Release)
- clang++ (Debug & Release)

**macOS:**
- clang++ (Debug & Release)

**Windows:**
- MSVC (Visual Studio 2022, Debug & Release)

**Compiler Version Matrix (Ubuntu 22.04):**
- GCC 11, 12, 13
- Clang 14, 15, 16

#### What it does:
1. Checks out the repository
2. Installs necessary dependencies (CMake, Ninja)
3. Configures the project using CMake
4. Builds the project
5. Runs unit tests using CTest

### 2. Code Quality Workflow (`code-quality.yml`)

This workflow performs static analysis and code quality checks.

#### Checks included:
- **Clang-Format**: Verifies code formatting matches the `.clang-format` file
- **Clang-Tidy**: Performs static analysis to find potential bugs and code smells
- **Cppcheck**: Additional static analysis tool for C++ code

## Usage

### Automatic Triggers

Both workflows automatically run on:
- Push to `master` or `main` branches
- Pull requests targeting `master` or `main` branches

### Manual Trigger

You can also manually trigger workflows from the GitHub Actions tab.

## Customization

### Adding More Compilers

To add more compiler versions, edit the `compiler-versions` job in `ci.yml`:

```yaml
- compiler: gcc
  version: 14
  cxx: g++-14
```

### Changing Build Configurations

Modify the `build_type` matrix to add configurations:

```yaml
build_type: [Debug, Release, RelWithDebInfo, MinSizeRel]
```

### Adjusting Static Analysis

Edit `code-quality.yml` to:
- Add suppressions to cppcheck
- Configure clang-tidy checks
- Modify formatting rules

## Notes

- The workflows use the latest GitHub Actions runners
- Dependencies are installed via package managers (apt, brew)
- All builds use Ninja generator on Linux/macOS for faster builds
- Windows builds use Visual Studio 2022 generator
- Tests run with `--output-on-failure` to show detailed error messages
