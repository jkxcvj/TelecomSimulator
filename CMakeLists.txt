cmake_minimum_required(VERSION 3.20)

project(TelecomSimulator LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)

FetchContent_MakeAvailable(googletest)

add_library(telecom_core
    src/User.cpp
    src/Network.cpp
    src/Call.cpp
)

target_include_directories(telecom_core
    PUBLIC
    include
)

add_executable(TelecomSimulator
    src/main.cpp
)

target_link_libraries(TelecomSimulator
    PRIVATE
    telecom_core
)

enable_testing()

add_executable(TelecomGTests
    tests/UserTests.cpp
    tests/CallTests.cpp
    tests/NetworkTests.cpp
)

target_link_libraries(TelecomGTests
    PRIVATE
    telecom_core
    GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(TelecomGTests)