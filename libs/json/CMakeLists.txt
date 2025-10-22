cmake_minimum_required(VERSION 3.15)
project(json_parser VERSION 1.0.0)


# Source files
file(GLOB JSON_SOURCES src/*.cpp)
file(GLOB JSON_HEADERS includes/*.hpp)

# Determine if building as standalone or as part of larger project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(JSON_STANDALONE ON)
else()
    set(JSON_STANDALONE OFF)
endif()

# Create library target
if(JSON_STANDALONE)
    # Standalone executable for testing
    add_executable(json_parser app.cpp ${JSON_SOURCES})
    target_compile_options(json_parser PRIVATE -g -O0)
else()
    # Library mode
    add_library(json_parser STATIC ${JSON_SOURCES})
    add_library(cppress::json_parser ALIAS json_parser)
endif()

# Link to common dependencies
target_link_libraries(json_parser PUBLIC cppress_common)

# Include directories
target_include_directories(json_parser PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/includes>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)


# Installation
if(NOT JSON_STANDALONE)
    install(TARGETS json_parser
        EXPORT cppressTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    
    install(DIRECTORY includes/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cppress/json
    )


    install(FILES includes.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cppress/json
    )

endif()



# Unit tests

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    enable_testing()
    add_subdirectory(tests)
endif()