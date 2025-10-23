cmake_minimum_required(VERSION 3.15)
project(cppress VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard globally
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options for building
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_TESTS "Build tests" ON)
option(BUILD_INTEGRATION_TESTS "Build integration tests" ON)

# Sanitizer support (set via -DSANITIZER=<type> from scripts.sh)
set(SANITIZER "" CACHE STRING "Sanitizer type (address, thread, undefined, memory, leak)")

# Global compiler flags
if(SANITIZER AND NOT WIN32)
    message(STATUS "Enabling sanitizer: ${SANITIZER}")
    
    # Map sanitizer types to compiler flags
    if(SANITIZER STREQUAL "address")
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g -O1)
        add_link_options(-fsanitize=address)
    elseif(SANITIZER STREQUAL "thread")
        add_compile_options(-fsanitize=thread -fno-omit-frame-pointer -g -O1)
        add_link_options(-fsanitize=thread)
    elseif(SANITIZER STREQUAL "undefined")
        add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer -g -O1)
        add_link_options(-fsanitize=undefined)
    elseif(SANITIZER STREQUAL "memory")
        add_compile_options(-fsanitize=memory -fno-omit-frame-pointer -g -O1)
        add_link_options(-fsanitize=memory)
    elseif(SANITIZER STREQUAL "leak")
        add_compile_options(-fsanitize=leak -fno-omit-frame-pointer -g -O1)
        add_link_options(-fsanitize=leak)
    else()
        message(WARNING "Unknown sanitizer type: ${SANITIZER}")
    endif()
endif()

# Platform-specific settings
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
    if(MSVC)
        add_compile_options(/W4 /utf-8)
    endif()
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Create interface library for shared dependencies
add_library(cppress_common INTERFACE)
target_include_directories(cppress_common INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/shared/includes>
    $<INSTALL_INTERFACE:include/cppress/shared>
)

# Add subdirectories in dependency order
add_subdirectory(shared)
add_subdirectory(libs/json)
add_subdirectory(libs/html)
add_subdirectory(libs/sockets)
# add_subdirectory(libs/http)
# add_subdirectory(libs/web)

if(BUILD_INTEGRATION_TESTS)
    enable_testing()
    add_subdirectory(integration)
endif()

# Install rules
include(GNUInstallDirs)

install(TARGETS cppress_common
    EXPORT cppressTargets
)

install(EXPORT cppressTargets
    FILE cppressTargets.cmake
    NAMESPACE cppress::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cppress
)

# Generate and install package config
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/cppressConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cppressConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cppressConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cppress
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/cppressConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/cppressConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cppress
)