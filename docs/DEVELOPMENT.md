# Development guide

Start with the [README quick start](../README.md#quick-start). Commands below assume Linux / WSL and run from the repository root.

## Build targets and options

| Target | Purpose |
|---|---|
| `telecom_core` | Reusable application library |
| `TelecomSimulator` | Interactive executable |
| `TelecomGTests` | GoogleTest test executable, discovered by CTest |
| `format` / `format-check` | Apply / verify `.clang-format` |
| `AsanDemo` / `UbsanDemo` | Deliberately faulty educational programs, enabled with the respective sanitizer |

| CMake option | Default | Purpose |
|---|---|---|
| `ENABLE_WARNINGS_AS_ERRORS` | `OFF` | Treat compiler warnings as errors |
| `ENABLE_ADDRESS_SANITIZER` | `OFF` | Instrument memory-access checks |
| `ENABLE_UNDEFINED_BEHAVIOR_SANITIZER` | `OFF` | Instrument undefined-behavior checks; unsupported by this project's MSVC configuration |
| `ENABLE_CLANG_TIDY` | `OFF` | Run clang-tidy during compilation |

GoogleTest is fetched during configuration. clang-format is also required at configuration time. For an offline build with an existing GoogleTest source checkout, pass `-DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=/absolute/path/to/googletest`.

## Formatting and static analysis

```bash
cmake --build build --target format-check
# Apply formatting when needed:
cmake --build build --target format

sudo apt-get install -y clang-tidy
cmake -S . -B build-tidy -DCMAKE_BUILD_TYPE=Debug \
  -DENABLE_CLANG_TIDY=ON -DENABLE_WARNINGS_AS_ERRORS=ON
cmake --build build-tidy --parallel 2
ctest --test-dir build-tidy --output-on-failure
```

## Sanitizers

```bash
cmake -S . -B build-sanitizers -DCMAKE_BUILD_TYPE=Debug \
  -DENABLE_WARNINGS_AS_ERRORS=ON \
  -DENABLE_ADDRESS_SANITIZER=ON \
  -DENABLE_UNDEFINED_BEHAVIOR_SANITIZER=ON
cmake --build build-sanitizers --parallel 2
ctest --test-dir build-sanitizers --output-on-failure
```

`AsanDemo` and `UbsanDemo` deliberately trigger sanitizer failures when executed manually. They are not part of the CTest suite.

Run a focused set of tests with, for example:

```bash
ctest --test-dir build -R 'Cli|Persistence' --output-on-failure
```

Tests use temporary files with shared names inside their working directory. Run CTest sequentially, as shown above, to avoid file collisions.

## Install and package

```bash
cmake --install build --prefix "$PWD/install"
./install/bin/TelecomSimulator
cpack --config build/CPackConfig.cmake -B build/packages
```

The CPack configuration creates a TGZ archive containing `bin/TelecomSimulator`. This is a native binary, so distribute it for a compatible operating system and C++ runtime. Build output, local installations, logs, and package archives are ignored by Git.

## Before opening a pull request

Build with warnings as errors, check formatting, run CTest, and replay `examples/demo-input.txt`. Update the documentation if commands, behavior, or file formats change. Add regression tests for behavior fixes.

The Release CI job also replays the demo and checks the busy-user rejection and final call states. Sanitizers and clang-tidy run in separate jobs.
