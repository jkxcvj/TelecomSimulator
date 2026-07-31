cmake_minimum_required(VERSION 3.20)

project(TelecomSimulator LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(
    ENABLE_WARNINGS_AS_ERRORS
    "Treat compiler warnings as errors"
    OFF
)

function(enable_project_warnings target_name)
    if(MSVC)
        target_compile_options(
            ${target_name}
            PRIVATE
            /W4
        )

        if(ENABLE_WARNINGS_AS_ERRORS)
            target_compile_options(
                ${target_name}
                PRIVATE
                /WX
            )
        endif()
    else()
        target_compile_options(
            ${target_name}
            PRIVATE
            -Wall
            -Wextra
            -Wpedantic
        )

        if(ENABLE_WARNINGS_AS_ERRORS)
            target_compile_options(
                ${target_name}
                PRIVATE
                -Werror
            )
        endif()
    endif()
endfunction()

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

enable_project_warnings(telecom_core)

add_executable(TelecomSimulator
    src/main.cpp
)

target_link_libraries(TelecomSimulator
    PRIVATE
    telecom_core
)

enable_project_warnings(TelecomSimulator)

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

enable_project_warnings(TelecomGTests)

include(GoogleTest)

gtest_discover_tests(TelecomGTests)