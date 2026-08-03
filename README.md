# TelecomSimulator

[![C++ CI](https://github.com/jkxcvj/TelecomSimulator/actions/workflows/ci.yml/badge.svg)](https://github.com/jkxcvj/TelecomSimulator/actions/workflows/ci.yml)

TelecomSimulator is a small C++20 application that models users and telephone calls inside a simple telecom network.

The project is developed as a practical learning environment for modern C++, API design, automated testing, CMake, static analysis, sanitizers, Git workflows, and continuous integration.

## Current Features

- Register users in the network
- Reject duplicate user IDs
- Create calls between registered users
- Start and end existing calls
- Track call states:
  - `Created`
  - `Active`
  - `Ended`
- Prevent a user from participating in more than one active call
- Store and display users and call records
- Validate the core network logic with automated GoogleTest tests
- Run formatting, static analysis, compiler warnings, and sanitizers locally and in CI
- Index users and calls by ID using `std::unordered_map`

## Example Output

```text
================================
      TELECOM SIMULATOR
================================

--- Registering users ---
[OK] Added user Alice with ID 1
[OK] Added user Bob with ID 2
[OK] Added user Charlie with ID 3
[ERROR] User with ID 1 already exists

--- Registered users ---
User ID: 1, Name: Alice Johnson
User ID: 2, Name: Bob Smith
User ID: 3, Name: Charlie Brown

--- Call scenario ---
[OK] Created call 101 from Alice to Bob
[OK] Started call 101 between Alice and Bob
[OK] Created call 102 from Alice to Charlie
[ERROR] Could not start call 102 because Alice is already busy
[OK] Ended call 101 between Alice and Bob

--- Final call records ---
Call ID: 101
Caller ID: 1
Receiver ID: 2
Status: Ended

Call ID: 102
Caller ID: 1
Receiver ID: 3
Status: Created

Demo completed. Registered users: 3, call records: 2
```

## Technologies and Tools

- C++20
- CMake
- GoogleTest
- Git and GitHub
- GitHub Actions
- clang-format
- clang-tidy
- AddressSanitizer
- UndefinedBehaviorSanitizer

## Project Structure

```text
.
├── .clang-format
├── .clang-tidy
├── .github/
│   └── workflows/
│       └── ci.yml
├── .vscode/
│   └── settings.json
├── examples/
│   ├── AsanDemo.cpp
│   ├── ManualTestRunner.cpp
│   └── UbsanDemo.cpp
├── include/
│   ├── Call.h
│   ├── Network.h
│   └── User.h
├── src/
│   ├── Call.cpp
│   ├── Network.cpp
│   ├── User.cpp
│   └── main.cpp
├── tests/
│   ├── CallTests.cpp
│   ├── NetworkTests.cpp
│   └── UserTests.cpp
├── CMakeLists.txt
└── README.md
```

### Main Directories

- `include/` — public project headers
- `src/` — production code and the demo application
- `tests/` — automated GoogleTest tests
- `examples/` — educational examples for manual testing and sanitizers
- `.github/workflows/` — GitHub Actions CI configuration

## Requirements

- CMake 3.20 or newer
- A compiler with C++20 support
- Git
- clang-format
- clang-tidy when `ENABLE_CLANG_TIDY=ON`
- Internet access during the first CMake configuration so GoogleTest can be downloaded through `FetchContent`

The commands below assume Linux or WSL with a single-config CMake generator.

## Building the Project

Configure a Release build:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

Build all default targets:

```bash
cmake --build build
```

## Running the Application

```bash
./build/TelecomSimulator
```

## Running Tests

```bash
ctest --test-dir build --output-on-failure
```

The test suite covers the main behavior of:

- `User`
- `Call`
- `Network`
- call creation
- call state transitions
- invalid operations
- busy-user protection

## Code Quality

### Compiler Warnings

The project enables:

- GCC and Clang:
  - `-Wall`
  - `-Wextra`
  - `-Wpedantic`
- MSVC:
  - `/W4`

Warnings can be promoted to errors:

```bash
cmake -S . -B build-werror \
    -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_WARNINGS_AS_ERRORS=ON

cmake --build build-werror
```

### Formatting

Format all project C++ files:

```bash
cmake --build build --target format
```

Check formatting without changing files:

```bash
cmake --build build --target format-check
```

The formatting rules are defined in `.clang-format`.

### Static Analysis

Configure a build with clang-tidy enabled:

```bash
cmake -S . -B build-tidy \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_CLANG_TIDY=ON \
    -DENABLE_WARNINGS_AS_ERRORS=ON
```

Build the project and run clang-tidy automatically:

```bash
cmake --build build-tidy
```

The enabled checks are configured in `.clang-tidy`.

### AddressSanitizer and UndefinedBehaviorSanitizer

Configure a sanitizer build:

```bash
cmake -S . -B build-sanitizers \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_WARNINGS_AS_ERRORS=ON \
    -DENABLE_ADDRESS_SANITIZER=ON \
    -DENABLE_UNDEFINED_BEHAVIOR_SANITIZER=ON
```

Build and test:

```bash
cmake --build build-sanitizers
ctest --test-dir build-sanitizers --output-on-failure
```

The `examples/` directory also contains small educational programs used to demonstrate sanitizer behavior.

## CMake Options

| Option | Default | Description |
|---|---:|---|
| `ENABLE_WARNINGS_AS_ERRORS` | `OFF` | Treat compiler warnings as errors |
| `ENABLE_ADDRESS_SANITIZER` | `OFF` | Enable AddressSanitizer |
| `ENABLE_UNDEFINED_BEHAVIOR_SANITIZER` | `OFF` | Enable UndefinedBehaviorSanitizer |
| `ENABLE_CLANG_TIDY` | `OFF` | Run clang-tidy during compilation |

## Continuous Integration

GitHub Actions runs automatically for pushes and pull requests targeting `main`.

The CI pipeline contains three independent jobs.

### Release Build

- Configures a Release build
- Treats compiler warnings as errors
- Checks clang-format compliance
- Builds the project
- Runs the full test suite

### Sanitizer Build

- Configures a Debug build
- Enables AddressSanitizer
- Enables UndefinedBehaviorSanitizer
- Treats compiler warnings as errors
- Builds the project
- Runs the full test suite

### Clang-Tidy

- Installs clang-tidy
- Enables static analysis during compilation
- Treats accepted diagnostics as errors
- Builds the project
- Runs the full test suite

## Architecture

The project currently separates the domain model from the executable demo:

- `User` represents a registered network user.
- `Call` stores call participants and manages the call state.
- `Network` owns users and call records and coordinates operations between them.
- `main.cpp` presents a readable demonstration scenario.
- GoogleTest tests validate the domain behavior independently from the demo output.

The code is built as a reusable `telecom_core` static library, which is linked by both the application and the test executable.

## Roadmap

Planned development modules include:

- Strong types for `UserId` and `CallId`
- RAII and ownership
- Smart pointers and dynamic polymorphism
- Copy and move semantics
- Consistent error handling
- Data persistence
- Interactive command-line interface
- Dependency management and packaging
- Docker and multi-stage builds
- Multithreading, synchronization, race conditions, and deadlocks
- Code coverage, benchmarks, profiling, and final portfolio preparation

## Learning Goals

This project is intentionally developed in small, reviewable steps. Its main learning goals are:

- Write maintainable modern C++
- Understand object lifetime and ownership
- Design safer and clearer APIs
- Select appropriate standard library containers
- Analyze time and space complexity
- Write automated tests for positive and negative scenarios
- Configure a multi-target CMake project
- Use compiler warnings and static analysis effectively
- Detect memory and undefined-behavior problems with sanitizers
- Work with feature branches, pull requests, code review, and CI
- Explain technical and architectural decisions during C++ interviews

## Status

The project is under active development. The current version focuses on the core telecom domain, automated tests, code quality tooling, and CI infrastructure.
