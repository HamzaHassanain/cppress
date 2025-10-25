cmake_minimum_required(VERSION 3.15)

# Use FetchContent to download and add GoogleTest
include(FetchContent)

# Configure GoogleTest to use the same runtime library as our project
if(WIN32 AND MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.12.1
)
FetchContent_MakeAvailable(googletest)

# Get all test files
file(GLOB TEST_FILES *_test.cpp)

# Only create test executable if there are test files
if(TEST_FILES)
    # Create a test executable
    add_executable(json-tests ${TEST_FILES})

    # Set C++17 standard for tests
    target_compile_features(json-tests PRIVATE cxx_std_17)

    # Windows-specific runtime library configuration
    if(WIN32 AND MSVC)
        set_property(TARGET json-tests PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    endif()

    # Link against all cppress libraries and GTest
    target_link_libraries(json-tests
        PRIVATE
            cppress_common
            json
            GTest::gtest_main
            GTest::gtest
    )


    # Tell CMake to find and register the tests to be run with CTest
    include(GoogleTest)
    gtest_discover_tests(json-tests)
    
    message(STATUS "Unit tests configured with ${TEST_FILES}")
else()
    message(STATUS "No unit test files found in tests/")
endif()
