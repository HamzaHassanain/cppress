cmake_minimum_required(VERSION 3.10)

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

# Get source files for the library (excluding main app)
file(GLOB PARSER_SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)

# Get all test files
file(GLOB TEST_FILES *_test.cpp)

# Create a test executable with the source files directly
add_executable(json_parser_tests ${TEST_FILES} ${PARSER_SRC_FILES})

# Include directories for headers
target_include_directories(json_parser_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/includes
    ${CMAKE_SOURCE_DIR}
)

# Set C++17 standard for tests
target_compile_features(json_parser_tests PRIVATE cxx_std_17)

# Windows-specific runtime library configuration
if(WIN32 AND MSVC)
    set_property(TARGET json_parser_tests PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Link your test executable to the GTest libraries only
# Note: We include source files directly to avoid linking issues with inline functions on Windows
target_link_libraries(json_parser_tests
  PRIVATE
    GTest::gtest_main
    GTest::gtest
)

# Tell CMake to find and register the tests to be run with CTest
include(GoogleTest)
gtest_discover_tests(json_parser_tests)
