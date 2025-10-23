cmake_minimum_required(VERSION 3.15)

# Use FetchContent to download and add GoogleTest
include(FetchContent)

# Configure GoogleTest to use the same runtime library as our project
if(WIN32 AND MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Pass sanitizer flags to GoogleTest if enabled
if(SANITIZER AND NOT WIN32)
    set(CMAKE_CXX_FLAGS_SAVE "${CMAKE_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_SAVE "${CMAKE_EXE_LINKER_FLAGS}")
endif()

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.12.1
)

# Set options before making available
set(gtest_disable_pthreads OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Restore flags if saved
if(SANITIZER AND NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_SAVE}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS_SAVE}")
endif()

# Get all test files
file(GLOB TEST_FILES *_test.cpp)

# Only create test executable if there are test files
if(TEST_FILES)
    # Create a test executable
    add_executable(sockets-tests ${TEST_FILES})

    # Set C++17 standard for tests
    target_compile_features(sockets-tests PRIVATE cxx_std_17)

    # Windows-specific runtime library configuration
    if(WIN32 AND MSVC)
        set_property(TARGET sockets-tests PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    endif()

    # Link against all cppress libraries and GTest
    target_link_libraries(sockets-tests
        PRIVATE
            cppress_common
            GTest::gtest_main
            GTest::gtest
    )

    target_link_libraries(sockets-tests PRIVATE sockets)

    # Tell CMake to find and register the tests to be run with CTest
    include(GoogleTest)
    gtest_discover_tests(sockets-tests)
    
    message(STATUS "Unit tests configured with ${TEST_FILES}")
else()
    message(STATUS "No unit test files found in tests/")
endif()
