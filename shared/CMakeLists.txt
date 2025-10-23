# Shared utilities and common dependencies

# Collect source files
file(GLOB SHARED_SOURCES src/*.cpp)
file(GLOB SHARED_HEADERS includes/*.hpp)

if(SHARED_SOURCES)

    add_library(cppress_shared_utils STATIC ${SHARED_SOURCES} ${SHARED_HEADERS})
    target_link_libraries(cppress_shared_utils PUBLIC cppress_common)
    target_include_directories(cppress_shared_utils PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/cppress/shared>
    )

    add_library(cppress::shared ALIAS cppress_shared_utils)

    install(TARGETS cppress_shared_utils
        EXPORT cppressTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    message(STATUS "Created cppress_shared_utils library with ${SHARED_SOURCES}")
else()
    if(SHARED_HEADERS)
        target_include_directories(cppress_common INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            $<INSTALL_INTERFACE:include/cppress/shared>
        )
        message(STATUS "Shared library is header-only with ${SHARED_HEADERS}")
    else()
        message(STATUS "No shared source or header files found")
    endif()
endif()

if(SHARED_HEADERS)
    install(DIRECTORY includes/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cppress/shared
        FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
    )
endif()