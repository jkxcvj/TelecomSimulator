cmake_minimum_required(VERSION 3.20)

project(TelecomSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

add_executable(TelecomTests
    tests/TestMain.cpp)

target_link_libraries(TelecomTests
    PRIVATE
    telecom_core
)

add_test(
    NAME TelecomTests
    COMMAND TelecomTests
)