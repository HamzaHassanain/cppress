
#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace cppress::json {

class json_object;

/**
 * @brief Parses a JSON value from a string.
 *
 * This function can parse any JSON value type including objects, arrays,
 * strings, numbers, booleans, and null values.
 *
 * @param valueString The JSON string to parse.
 * @return A shared pointer to the parsed json_object, or nullptr if parsing fails.
 *
 * @note This function handles all JSON value types:
 *       - Objects: {...}
 *       - Arrays: [...]
 *       - Strings: "..."
 *       - Numbers: 123, 45.67, -8.9e10
 *       - Booleans: true, false
 *       - Null: null
 *
 * @example
 * @code
 * auto str_val = cppress::json_value("\"hello\"");
 * auto num_val = cppress::json_value("42");
 * auto obj_val = cppress::json_value("{\"key\": \"value\"}");
 * @endcode
 */
std::shared_ptr<json_object> json_value(const std::string& valueString);

/**
 * @brief Parses a complete JSON object from a string.
 *
 * This is the main parsing function that expects a JSON object at the root level.
 * It processes the JSON string, removes comments, and normalizes whitespace before
 * parsing.
 *
 * @param jsonString The JSON object string to parse (must start with '{').
 * @return An unordered_map containing the key-value pairs of the root object.
 * @throws std::runtime_error if the JSON is malformed or doesn't start with an object.
 *
 * @note This function:
 *       - Trims leading/trailing whitespace
 *       - Removes C-style comments (//)
 *       - Removes whitespace outside of string literals
 *       - Requires the root to be a JSON object
 *
 * @example
 * @code
 * std::string json = R"({
 *     "name": "John",
 *     "age": 30,
 *     "active": true
 * })";
 * auto data = cppress::parse(json);
 * @endcode
 */
std::unordered_map<std::string, std::shared_ptr<json_object>> parse(const std::string& jsonString);

}  // namespace cppress
