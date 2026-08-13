cmake_minimum_required(VERSION 3.20)

project(TelecomSimulator LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(
    ENABLE_WARNINGS_AS_ERRORS
    "Treat compiler warnings as errors"
    OFF
)

option(
    ENABLE_ADDRESS_SANITIZER
    "Enable AddressSanitizer"
    OFF
)

option(
    ENABLE_UNDEFINED_BEHAVIOR_SANITIZER
    "Enable UndefinedBehaviorSanitizer"
    OFF
)

option(
    ENABLE_CLANG_TIDY
    "Run clang-tidy during compilation"
    OFF
)

if(ENABLE_CLANG_TIDY)
    find_program(
        CLANG_TIDY_EXECUTABLE
        NAMES clang-tidy
        REQUIRED
    )
endif()

function(enable_clang_tidy target_name)
    if(ENABLE_CLANG_TIDY)
        set_target_properties(
            ${target_name}
            PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_EXECUTABLE};--quiet"
        )
    endif()
endfunction()

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

function(enable_address_sanitizer target_name)
    if(NOT ENABLE_ADDRESS_SANITIZER)
        return()
    endif()

    if(MSVC)
        target_compile_options(
            ${target_name}
            PRIVATE
            /fsanitize=address
        )
    else()
        target_compile_options(
            ${target_name}
            PRIVATE
            -fsanitize=address
            -fno-omit-frame-pointer
        )

        get_target_property(target_type ${target_name} TYPE)

        if(NOT target_type STREQUAL "STATIC_LIBRARY")
            target_link_options(
                ${target_name}
                PRIVATE
                -fsanitize=address
            )
        endif()
    endif()
endfunction()

function(enable_undefined_behavior_sanitizer target_name)
    if(NOT ENABLE_UNDEFINED_BEHAVIOR_SANITIZER)
        return()
    endif()

    if(MSVC)
        message(WARNING
            "UndefinedBehaviorSanitizer is not supported for MSVC in this project"
        )
        return()
    endif()

    target_compile_options(
        ${target_name}
        PRIVATE
        -fsanitize=undefined
        -fno-sanitize-recover=undefined
        -fno-omit-frame-pointer
    )

    get_target_property(target_type ${target_name} TYPE)

    if(NOT target_type STREQUAL "STATIC_LIBRARY")
        target_link_options(
            ${target_name}
            PRIVATE
            -fsanitize=undefined
        )
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
    src/ConsoleEventLogger.cpp
    src/FileEventLogger.cpp
    src/EventDispatcher.cpp
    src/IntBuffer.cpp
)

target_include_directories(telecom_core
    PUBLIC
    include
)

enable_project_warnings(telecom_core)
enable_address_sanitizer(telecom_core)
enable_undefined_behavior_sanitizer(telecom_core)
enable_clang_tidy(telecom_core)

add_executable(TelecomSimulator
    src/main.cpp
)

target_link_libraries(TelecomSimulator
    PRIVATE
    telecom_core
)

enable_project_warnings(TelecomSimulator)
enable_address_sanitizer(TelecomSimulator)
enable_undefined_behavior_sanitizer(TelecomSimulator)
enable_clang_tidy(TelecomSimulator)

enable_testing()

add_executable(TelecomGTests
    tests/UserTests.cpp
    tests/CallTests.cpp
    tests/NetworkTests.cpp
    tests/IdentifierTests.cpp
    tests/OwnershipTests.cpp
    tests/FileEventLogger.cpp
    tests/EventDispatcherTests.cpp
    tests/SpecialMemberTests.cpp
    tests/ExceptionSafetyTests.cpp
    tests/IteratorTests.cpp
    tests/TemplateTests.cpp
    tests/RegistryTests.cpp
    tests/LimitedRegistryTests.cpp
)

target_link_libraries(TelecomGTests
    PRIVATE
    telecom_core
    GTest::gtest_main
)

enable_project_warnings(TelecomGTests)
enable_address_sanitizer(TelecomGTests)
enable_undefined_behavior_sanitizer(TelecomGTests)
enable_clang_tidy(TelecomGTests)

include(GoogleTest)

gtest_discover_tests(TelecomGTests)

if(ENABLE_ADDRESS_SANITIZER)
    add_executable(AsanDemo
        examples/AsanDemo.cpp
    )

    enable_project_warnings(AsanDemo)
    enable_address_sanitizer(AsanDemo)
endif()

if(ENABLE_UNDEFINED_BEHAVIOR_SANITIZER)
    add_executable(UbsanDemo
        examples/UbsanDemo.cpp
    )

    enable_project_warnings(UbsanDemo)
    enable_undefined_behavior_sanitizer(UbsanDemo)
endif()

find_program(
    CLANG_FORMAT_EXECUTABLE
    NAMES clang-format
    REQUIRED
)

file(
    GLOB_RECURSE PROJECT_FORMAT_FILES
    CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/include/*.h"
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/tests/*.cpp"
    "${CMAKE_SOURCE_DIR}/examples/*.cpp"
)

add_custom_target(
    format
    COMMAND
    ${CLANG_FORMAT_EXECUTABLE}
    -i
    ${PROJECT_FORMAT_FILES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Formatting C++ source files"
    VERBATIM
)

add_custom_target(
    format-check
    COMMAND
    ${CLANG_FORMAT_EXECUTABLE}
    --dry-run
    --Werror
    ${PROJECT_FORMAT_FILES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Checking C++ source formatting"
    VERBATIM
)
