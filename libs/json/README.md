# Simple JSON Parser

A simple JSON parser written in C++.

### Check app.cpp for example usage

## Table of Contents

- [Prerequisites](#prerequisites)

  - [For Linux (Ubuntu/Debian)](#for-linux-ubuntudebian)
  - [For Linux (CentOS/RHEL/Fedora)](#for-linux-centosrhelfedora)
  - [For Windows](#for-windows)

- [Quick Start](#quick-start)

  - [Using Git Submodules](#using-git-submodules)

- [API Documentation](#api-documentation)

## Prerequisites

- CMake 3.10 or higher
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)

## Quick Start

### Using Git Submodules

You just need to clone the repository as a submodule:

```bash
# In your base project directory, run the following command
git submodule add https://github.com/HamzaHassanain/hamza-json-parser.git ./submodules/json-parser
```

Then in your project's CMakeLists.txt, include the submodule:

```cmake
# Your project's CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(my_project)

# This block checks for Git and initializes submodules recursively


if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")

    # Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    option(GIT_SUBMODULE_UPDATE_LATEST "Update submodules to latest remote commits" ON)


    if(GIT_SUBMODULE)
        message(STATUS "Initializing and updating submodules...")

        # First, initialize submodules if they don't exist
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_INIT_RESULT)
        if(NOT GIT_SUBMOD_INIT_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_INIT_RESULT}, please checkout submodules")
        endif()

        # If enabled, update submodules to latest remote commits
        if(GIT_SUBMODULE_UPDATE_LATEST)
            message(STATUS "Updating submodules to latest remote commits...")
            execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --remote --recursive
                            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                            RESULT_VARIABLE GIT_SUBMOD_UPDATE_RESULT)
            if(NOT GIT_SUBMOD_UPDATE_RESULT EQUAL "0")
                message(WARNING "git submodule update --remote --recursive failed with ${GIT_SUBMOD_UPDATE_RESULT}, continuing with current submodule versions")
            else()
                message(STATUS "Submodules updated to latest versions successfully")
            endif()
        endif()
    endif()
endif()


# Add the submodule
add_subdirectory(submodules/json-parser)

# Include directories for the library headers (this will also suppress warnings from headers)
target_include_directories(my_project SYSTEM PRIVATE submodules/json-parser/include)

# Add additional compiler flags to suppress warnings from library headers
target_compile_options(my_project PRIVATE -Wno-comment -Wno-overloaded-virtual -Wno-reorder)

# Link against the main library
# The library's CMakeLists.txt handles linking all dependencies automatically
target_link_libraries(my_project json_parser)
```

Then in your cpp file, include the http library header:

```cpp
#include "submodules/json-parser/json-parser.hpp"
```

## Running Tests

This project includes Google Test for unit testing. To build and run the tests:

```bash
# Navigate to the tests directory
cd tests

# Configure the build
cmake -B build -S .

# Build the tests
cmake --build build

# Run the tests
./build/json_parser_tests

# Or use CTest to run tests
cd build && ctest --output-on-failure
```

The test suite includes comprehensive tests for:

- JSON object creation and manipulation
- JSON parsing functionality
- Helper functions (makers and getters)
- Error handling for invalid JSON

## API Documentation

Below is a short reference for the main public headers in this project. Each entry contains purpose, features, inheritance and the most important public API (signatures) so you can quickly discover how to use the library.

For more details, please refer to the individual header files.

#### hh_json::JsonObject

```cpp
#include "JsonObject.hpp"

// - Purpose: The base type for all JSON value types. Represents a JSON object (map of key -> JsonObject).
// - Features: Storage for named properties, stringify to JSON text, basic manipulation API.
// - Inheritance: Base class for concrete JSON types (JsonString, JsonNumber, JsonBoolean, JsonArray).
// - Key methods:
  JsonObject();                                           // — Constructor
  JsonObject(const std::unordered_map<std::string, std::shared_ptr<JsonObject>> &initial_data); // — Constructor with initial data
  virtual ~JsonObject();                                  // — Virtual destructor
  virtual bool set_json_data(const std::string &jsonString); // — Populate this object by parsing a JSON string
  virtual void insert(const std::string &key, std::shared_ptr<JsonObject> value); // — Insert/replace a property
  virtual void erase(const std::string &key);             // — Remove a property
  virtual std::shared_ptr<JsonObject> get(const std::string &key) const; // — Retrieve a property or nullptr
  virtual std::string stringify() const;                  // — Serialize to JSON text
  virtual void clear();                                   // — Remove all properties
  const std::unordered_map<std::string, std::shared_ptr<JsonObject>> &get_data() const; // — Get underlying data
  std::shared_ptr<JsonObject> &operator[](const std::string &key); // — Convenience accessor
  bool has_key(const std::string &key) const;            // — Check if key exists
```

#### hh_json::JsonString

```cpp
#include "JsonString.hpp"

// - Purpose: Represents a JSON string value.
// - Features: Stores the string content and serializes with surrounding quotes and necessary escapes.
// - Inheritance: public hh_json::JsonObject
// - Key methods:
  JsonString();
  JsonString(const std::string &value);                  // — Construct from std::string
  bool set_json_data(const std::string &jsonString) override; // — Set string value (used by parser)
  std::string stringify() const override;                 // — Returns quoted/escaped string
```

#### hh_json::JsonNumber

```cpp
#include "JsonNumber.hpp"

// - Purpose: Represents a numeric JSON value (stored as double).
// - Features: Parses numeric text to double and formats numbers with std::to_string on stringify.
// - Inheritance: public hh_json::JsonObject
// - Key methods:
  JsonNumber();
  JsonNumber(double value);                              // — Construct from a numeric value
  bool set_json_data(const std::string &jsonString) override; // — Parse numeric literal
  std::string stringify() const override;                 // — Serialize number
```

#### hh_json::JsonBoolean

```cpp
#include "JsonBoolean.hpp"

// - Purpose: Represents a boolean JSON value.
// - Features: Case-insensitive parsing from text "true"/"false" and simple stringify.
// - Inheritance: public hh_json::JsonObject
// - Key methods:
  JsonBoolean();
  JsonBoolean(bool value);                               // — Construct from bool
  bool set_json_data(const std::string &jsonString) override; // — Parse "true"/"false"
  std::string stringify() const override;                 // — Returns "true" or "false"
```

#### hh_json::JsonArray

```cpp
#include "JsonArray.hpp"

// - Purpose: Represents a JSON array (ordered list of JsonObject pointers).
// - Features: Holds elements, supports insertion and stringify for arrays.
// - Inheritance: public hh_json::JsonObject
// - Key methods:
  JsonArray();
  JsonArray(const std::vector<std::shared_ptr<JsonObject>> &elements); // — Construct from a vector
  void insert(std::shared_ptr<JsonObject> value);         // — Append element to array
  bool set_json_data(const std::string &jsonString) override; // — (Not implemented) parse array string
  std::string stringify() const override;                 // — Serialize array
```

#### hh_json::parser (parser.hpp)

```cpp
#include "parser.hpp"

// - Purpose: Parse a JSON text into the library's object representation.
// - Features: Returns an unordered_map representing the top-level object properties, plus a utility function for parsing single values.
// - Key functions:
  std::shared_ptr<JsonObject> JsonValue(const std::string &valueString); // — Parse a single JSON value
  std::unordered_map<std::string, std::shared_ptr<JsonObject>> parse(const std::string &jsonString); // — Parse full JSON object
// - Notes: The parser supports objects, arrays, strings, numbers, booleans and null. It performs a single-pass style parse and returns an in-memory representation using the hh_json types.
```

#### helpers.hpp (factory & getters)

```cpp
#include "helpers.hpp"

// - Purpose: Small convenience factory and getter helpers for creating and extracting typed JSON objects.
// - Features: Factory helpers (make_string/make_number/make_boolean) and getters that extract primitive values with runtime type checks.
// - Namespaces and key functions:
  namespace hh_json::maker
    std::shared_ptr<hh_json::JsonObject> make_string(const std::string &value);
    std::shared_ptr<hh_json::JsonObject> make_number(double value);
    std::shared_ptr<hh_json::JsonObject> make_boolean(bool value);

  namespace hh_json::getter
    bool get_boolean(const std::shared_ptr<hh_json::JsonObject> &obj);
    double get_number(const std::shared_ptr<hh_json::JsonObject> &obj);
    std::string get_string(const std::shared_ptr<hh_json::JsonObject> &obj);
    std::vector<std::shared_ptr<hh_json::JsonObject>> get_array(const std::shared_ptr<hh_json::JsonObject> &obj);

// - Notes: Getters perform dynamic casts and throw on type mismatch. Factories return JsonObject pointers to the concrete typed instances.
```
