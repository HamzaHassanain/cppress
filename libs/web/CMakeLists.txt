cmake_minimum_required(VERSION 3.15)
project(web_framework VERSION 1.0.0)


# Source files
file(GLOB WEB_SOURCES src/*.cpp)
file(GLOB WEB_HEADERS includes/*.hpp)


# Create library target
if(WEB_STANDALONE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -fsanitize=address -g -O0")
    
    # Create cppress_common interface library FIRST (required by subdirectories)
    add_library(cppress_common INTERFACE)
    target_include_directories(cppress_common INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/../../shared/includes
    )
    
    # Add dependencies in order: shared first (provides utils), then sockets (depends on shared)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../shared ${CMAKE_CURRENT_BINARY_DIR}/shared_build)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../sockets ${CMAKE_CURRENT_BINARY_DIR}/sockets_build)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../http ${CMAKE_CURRENT_BINARY_DIR}/http_build)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../json ${CMAKE_CURRENT_BINARY_DIR}/json_build)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../html ${CMAKE_CURRENT_BINARY_DIR}/html_build)
    
    # Standalone executable for testing
    add_executable(web_framework app.cpp ${WEB_SOURCES})
    
    # Link libraries in dependency order
    target_link_libraries(web_framework PUBLIC sockets cppress_shared_utils http json html)
    
    # Apply sanitizer flags if enabled
    if(SANITIZER AND NOT WIN32 AND SANITIZER_FLAGS)
        target_compile_options(web_framework PRIVATE ${SANITIZER_FLAGS})
        target_link_options(web_framework PRIVATE ${SANITIZER_LINK_FLAGS})
        message(STATUS "Applied sanitizer flags to standalone web_framework executable")
    endif()
else()
    # Library mode
    add_library(web_framework STATIC ${WEB_SOURCES})
    add_library(cppress::web_framework ALIAS web_framework)
    
    # Link to common dependencies (in library mode, cppress_common is provided by parent)
    target_link_libraries(web_framework PUBLIC cppress_common)
    
    # Apply sanitizer flags if enabled
    if(SANITIZER AND NOT WIN32 AND SANITIZER_FLAGS)
        target_compile_options(web_framework PUBLIC ${SANITIZER_FLAGS})
        target_link_options(web_framework PUBLIC ${SANITIZER_LINK_FLAGS})
        message(STATUS "Applied sanitizer flags to web_framework library")
    endif()
endif()

target_include_directories(web_framework PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../>
    $<INSTALL_INTERFACE:include>
)



# Installation
if(NOT WEB_STANDALONE)
    install(TARGETS web_framework
        EXPORT cppressTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    
    install(DIRECTORY includes/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cppress/web_framework
    )


    install(FILES includes.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cppress/web_framework
    )

endif()


# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
#     enable_testing()
#     add_subdirectory(tests)
# endif()



