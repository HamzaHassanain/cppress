# Shared utilities and common dependencies

# Add common utility sources if any
file(GLOB SHARED_SOURCES src/*.cpp)

if(SHARED_SOURCES)
    add_library(cppress_shared_utils STATIC ${SHARED_SOURCES})
    target_link_libraries(cppress_shared_utils PUBLIC cppress_common)
    target_include_directories(cppress_shared_utils PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/includes>
        $<INSTALL_INTERFACE:include>
    )
    
    install(TARGETS cppress_shared_utils
        EXPORT cppressTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()

# Install shared headers
install(DIRECTORY includes/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cppress/shared
)