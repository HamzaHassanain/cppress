cmake_minimum_required(VERSION 3.10)
project(json_parser)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Windows-specific runtime library configuration
if(WIN32 AND MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Load .env file and set JSON_LOCAL_TEST variable
# Function to read .env file and set variables
function(load_env_file env_file)
    if(EXISTS ${env_file})
        file(READ ${env_file} ENV_CONTENT)
        string(REPLACE "\n" ";" ENV_LINES ${ENV_CONTENT})
        
        foreach(line ${ENV_LINES})
            # Skip empty lines and comments
            if(line AND NOT line MATCHES "^#")
                string(FIND ${line} "=" eq_pos)
                if(eq_pos GREATER -1)
                    string(SUBSTRING ${line} 0 ${eq_pos} var_name)
                    math(EXPR val_start "${eq_pos} + 1")
                    string(SUBSTRING ${line} ${val_start} -1 var_value)
                    
                    # Remove any whitespace
                    string(STRIP ${var_name} var_name)
                    string(STRIP ${var_value} var_value)
                    
                    # Set the variable in parent scope
                    set(${var_name} ${var_value} PARENT_SCOPE)
                    message(STATUS "Loaded env variable: ${var_name}=${var_value}")
                endif()
            endif()
        endforeach()
    else()
        message(WARNING ".env file not found at ${env_file}")
    endif()
endfunction()

load_env_file(${CMAKE_SOURCE_DIR}/.env)

if(JSON_LOCAL_TEST AND JSON_LOCAL_TEST STREQUAL "1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -fsanitize=address -g -O0")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

file(GLOB SRC_FILES src/*.cpp)

# Check environment variable
if( JSON_LOCAL_TEST AND JSON_LOCAL_TEST STREQUAL "1")
    message(STATUS "Building as executable (JSON_LOCAL_TEST=1)")
    add_executable(json_parser app.cpp ${SRC_FILES})
    target_compile_features(json_parser PRIVATE cxx_std_17)
else()
    message(STATUS "Building as static library (JSON_LOCAL_TEST!=1)")
    add_library(json_parser STATIC ${SRC_FILES})
    target_compile_features(json_parser PRIVATE cxx_std_17)
    
    # Only enable testing when building as library
    enable_testing()
    # Include the test subdirectory
    add_subdirectory(tests)
endif()