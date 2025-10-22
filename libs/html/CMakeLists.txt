cmake_minimum_required(VERSION 3.15)
project(html VERSION 1.0.0)


# Source files
file(GLOB HTML_SOURCES src/*.cpp)
file(GLOB HTML_HEADERS includes/*.hpp)

# Determine if building as standalone or as part of larger project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(HTML_STANDALONE ON)
else()
    set(HTML_STANDALONE OFF)
endif()

# Create library target
if(HTML_STANDALONE)
    # Standalone executable for testing
    add_executable(html app.cpp ${HTML_SOURCES})
    target_compile_options(html PRIVATE -g -O0)
else()
    # Library mode
    add_library(html STATIC ${HTML_SOURCES})
    add_library(cppress::html ALIAS html)
endif()

# Link to common dependencies
target_link_libraries(html PUBLIC cppress_common)

# Include directories
target_include_directories(html PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/includes>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)



# Installation
if(NOT HTML_STANDALONE)
    install(TARGETS html
        EXPORT cppressTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    
    install(DIRECTORY includes/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cppress/html
    )


    install(FILES includes.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cppress/html
    )

endif()


if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    enable_testing()
    add_subdirectory(tests)
endif()