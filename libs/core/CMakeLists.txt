cmake_minimum_required(VERSION 3.15)
project(core VERSION 1.0.0)


# Source files
file(GLOB CORE_SOURCES src/*.cpp)
file(GLOB CORE_HEADERS includes/*.hpp)
file(GLOB CORE_MIDDLEWARES middlewares/*.hpp)


# Create library target
if(CORE_STANDALONE)
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
    add_executable(core app.cpp ${CORE_SOURCES})
    
    # Link libraries in dependency order
    target_link_libraries(core PUBLIC sockets cppress_shared_utils)
    
    # Apply sanitizer flags if enabled
    if(SANITIZER AND NOT WIN32 AND SANITIZER_FLAGS)
        target_compile_options(core PRIVATE ${SANITIZER_FLAGS})
        target_link_options(core PRIVATE ${SANITIZER_LINK_FLAGS})
        message(STATUS "Applied sanitizer flags to standalone core executable")
    endif()
else()
    # Library mode
    add_library(core STATIC ${CORE_SOURCES})
    add_library(cppress::core ALIAS core)
    
    # Link to common dependencies (in library mode, cppress_common is provided by parent)
    target_link_libraries(core PUBLIC cppress_common)
    
    # Apply sanitizer flags if enabled
    if(SANITIZER AND NOT WIN32 AND SANITIZER_FLAGS)
        target_compile_options(core PUBLIC ${SANITIZER_FLAGS})
        target_link_options(core PUBLIC ${SANITIZER_LINK_FLAGS})
        message(STATUS "Applied sanitizer flags to core library")
    endif()
endif()




# Installation
if(NOT CORE_STANDALONE)
    install(TARGETS core
        EXPORT cppressTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    
    install(DIRECTORY includes/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cppress/core
    )


    install(FILES includes.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cppress/core
    )

endif()


if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    enable_testing()
    add_subdirectory(tests)
endif()



