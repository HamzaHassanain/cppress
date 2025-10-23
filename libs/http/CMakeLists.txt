cmake_minimum_required(VERSION 3.15)
project(http VERSION 1.0.0)


# Source files
file(GLOB HTTP_SOURCES src/*.cpp)
file(GLOB HTTP_HEADERS includes/*.hpp)


# Create library target
if(HTTP_STANDALONE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -fsanitize=address -g -O0")
    
    # Create cppress_common interface library FIRST (required by subdirectories)
    add_library(cppress_common INTERFACE)
    target_include_directories(cppress_common INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/../../shared/includes
    )
    
    # Add dependencies in order: shared first (provides utils), then sockets (depends on shared)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../shared ${CMAKE_CURRENT_BINARY_DIR}/shared_build)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../sockets ${CMAKE_CURRENT_BINARY_DIR}/sockets_build)
    
    # Standalone executable for testing
    add_executable(http app.cpp ${HTTP_SOURCES})
    
    # Link libraries in dependency order
    target_link_libraries(http PUBLIC sockets cppress_shared_utils)
    
    # Apply sanitizer flags if enabled
    if(SANITIZER AND NOT WIN32 AND SANITIZER_FLAGS)
        target_compile_options(http PRIVATE ${SANITIZER_FLAGS})
        target_link_options(http PRIVATE ${SANITIZER_LINK_FLAGS})
        message(STATUS "Applied sanitizer flags to standalone http executable")
    endif()
else()
    # Library mode
    add_library(http STATIC ${HTTP_SOURCES})
    add_library(cppress::http ALIAS http)
    
    # Link to common dependencies (in library mode, cppress_common is provided by parent)
    target_link_libraries(http PUBLIC cppress_common)
    
    # Apply sanitizer flags if enabled
    if(SANITIZER AND NOT WIN32 AND SANITIZER_FLAGS)
        target_compile_options(http PUBLIC ${SANITIZER_FLAGS})
        target_link_options(http PUBLIC ${SANITIZER_LINK_FLAGS})
        message(STATUS "Applied sanitizer flags to http library")
    endif()
endif()

target_include_directories(http PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../>
    $<INSTALL_INTERFACE:include>
)



# Installation
if(NOT HTTP_STANDALONE)
    install(TARGETS http
        EXPORT cppressTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    
    install(DIRECTORY includes/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cppress/http
    )


    install(FILES includes.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cppress/http
    )

endif()


if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests AND NOT HTTP_STANDALONE)
    enable_testing()
    add_subdirectory(tests)
endif()



