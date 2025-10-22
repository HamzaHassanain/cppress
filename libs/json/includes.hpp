/**
 * @file includes.hpp
 * @brief Main header file for the cppress JSON library.
 *
 * This library provides a complete JSON parser and manipulation framework with
 * an STL-like interface. It supports all JSON data types and provides type-safe
 * operations with familiar C++ Standard Library patterns.
 *
 * @section features Features
 * - Full JSON parsing and serialization
 * - STL-like container interfaces (similar to std::unordered_map, std::vector, std::string)
 * - Type-safe value access with optional error handling
 * - Iterator support for objects and arrays
 * - Comprehensive type checking utilities
 * - Doxygen-documented API
 *
 * @section types JSON Types
 * - json_object: Represents JSON objects with map-like interface
 * - json_array: Represents JSON arrays with vector-like interface
 * - json_string: Represents JSON strings with string-like interface
 * - json_number: Represents JSON numbers with numeric operations
 * - json_boolean: Represents JSON boolean values
 *
 * @section namespaces Namespaces
 * - cppress: Main namespace containing all JSON types
 * - cppress::maker: Factory functions for creating JSON values
 * - cppress::getter: Type-safe accessor functions for extracting values
 *
 * @section example Basic Usage Example
 * @code
 * #include "includes.hpp"
 * using namespace cppress::json;
 *
 * // Parse JSON
 * std::string json_str = R"({"name": "John", "age": 30, "active": true})";
 * auto data = parse(json_str);
 *
 * // Access values with type safety
 * auto name = getter::get_string(data["name"]);
 * auto age = getter::get_number(data["age"]);
 * auto active = getter::get_boolean(data["active"]);
 *
 * // Create JSON values
 * auto obj = maker::make_object();
 * obj->insert("key", maker::make_string("value"));
 * obj->insert("count", maker::make_number(42));
 *
 * // STL-like iteration
 * for (const auto& [key, value] : *obj) {
 *     std::cout << key << ": " << value->stringify() << std::endl;
 * }
 *
 * // Array operations
 * auto arr = maker::make_array();
 * arr->push_back(maker::make_string("item1"));
 * arr->push_back(maker::make_number(123));
 * for (const auto& elem : *arr) {
 *     std::cout << elem->stringify() << std::endl;
 * }
 * @endcode
 *
 * @author Hamza Moahmmed Hassanain
 * @version 1.0
 */

#pragma once

#include "includes/healpers.hpp"
#include "includes/json_array.hpp"
#include "includes/json_boolean.hpp"
#include "includes/json_number.hpp"
#include "includes/json_object.hpp"
#include "includes/json_string.hpp"
#include "includes/parser.hpp"