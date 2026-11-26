cmake_minimum_required(VERSION 3.16)

project(RedisClone
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

add_executable(redis_clone
    src/main.cpp
)

if(MSVC)
    target_compile_options(redis_clone PRIVATE /W4)
else()
    target_compile_options(redis_clone PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(BUILD_TESTING)
    include(FetchContent)

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.16.0.zip
    )

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)

    add_executable(redis_clone_tests
        tests/test_main.cpp
    )

    target_link_libraries(redis_clone_tests
        PRIVATE
            GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(redis_clone_tests)
endif()
