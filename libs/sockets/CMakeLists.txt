cmake_minimum_required(VERSION 3.15)
project(sockets VERSION 1.0.0)


# Source files
file(GLOB SOCKET_SOURCES src/*.cpp)
file(GLOB SOCKET_HEADERS includes/*.hpp)

# Determine if building as standalone or as part of larger project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(SOCKET_STANDALONE ON)
else()
    set(SOCKET_STANDALONE OFF)
endif()

# Create library target
if(SOCKET_STANDALONE)
    # Standalone executable for testing
    add_executable(sockets app.cpp ${SOCKET_SOURCES})
    target_compile_options(sockets PRIVATE -g -O0)
else()
    # Library mode
    add_library(sockets STATIC ${SOCKET_SOURCES})
    add_library(cppress::sockets ALIAS sockets)
endif()

# Link to common dependencies
target_link_libraries(sockets PUBLIC cppress_common)

# Include directories
target_include_directories(sockets PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/includes>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)



# Installation
if(NOT SOCKET_STANDALONE)
    install(TARGETS sockets
        EXPORT cppressTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    
    install(DIRECTORY includes/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cppress/sockets
    )


    install(FILES includes.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cppress/sockets
    )

endif()


if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    enable_testing()
    add_subdirectory(tests)
endif()