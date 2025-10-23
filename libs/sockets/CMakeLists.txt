cmake_minimum_required(VERSION 3.15)
project(sockets VERSION 1.0.0)


# Source files
file(GLOB SOCKET_SOURCES src/*.cpp)
file(GLOB SOCKET_HEADERS includes/*.hpp)


# Create library target
if(SOCKET_STANDALONE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -fsanitize=address -g -O0")
    # Standalone executable for testing
    add_executable(sockets app.cpp ${SOCKET_SOURCES})
    # target_compile_options(sockets PRIVATE -Wall -Wextra -pedantic -fsanitize=address -g -O0)
    
    # Apply sanitizer flags if enabled
    if(SANITIZER AND NOT WIN32 AND SANITIZER_FLAGS)
        target_compile_options(sockets PRIVATE ${SANITIZER_FLAGS})
        target_link_options(sockets PRIVATE ${SANITIZER_LINK_FLAGS})
        message(STATUS "Applied sanitizer flags to standalone sockets executable")
    endif()
else()
    # Library mode
    add_library(sockets STATIC ${SOCKET_SOURCES})
    add_library(cppress::sockets ALIAS sockets)
    # Link to common dependencies
    target_link_libraries(sockets PUBLIC cppress_common)
    
    # Apply sanitizer flags if enabled
    if(SANITIZER AND NOT WIN32 AND SANITIZER_FLAGS)
        target_compile_options(sockets PUBLIC ${SANITIZER_FLAGS})
        target_link_options(sockets PUBLIC ${SANITIZER_LINK_FLAGS})
        message(STATUS "Applied sanitizer flags to sockets library")
    endif()
endif()

# Include directories
target_include_directories(sockets PUBLIC
    # $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/includes>
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


if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests AND NOT SOCKET_STANDALONE)
    enable_testing()
    add_subdirectory(tests)
endif()