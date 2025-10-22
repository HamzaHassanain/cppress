#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "json_array.hpp"
#include "json_boolean.hpp"
#include "json_number.hpp"
#include "json_object.hpp"
#include "json_string.hpp"

namespace cppress::json {

// Forward declarations for type checking
/**
 * @brief Checks if a json_object is a string.
 * @param obj The object to check.
 * @return true if the object is a json_string, false otherwise.
 */
inline bool is_string(const std::shared_ptr<json_object>& obj) {
    return std::dynamic_pointer_cast<json_string>(obj) != nullptr;
}

/**
 * @brief Checks if a json_object is a number.
 * @param obj The object to check.
 * @return true if the object is a json_number, false otherwise.
 */
inline bool is_number(const std::shared_ptr<json_object>& obj) {
    return std::dynamic_pointer_cast<json_number>(obj) != nullptr;
}

/**
 * @brief Checks if a json_object is a boolean.
 * @param obj The object to check.
 * @return true if the object is a json_boolean, false otherwise.
 */
inline bool is_boolean(const std::shared_ptr<json_object>& obj) {
    return std::dynamic_pointer_cast<json_boolean>(obj) != nullptr;
}

/**
 * @brief Checks if a json_object is an array.
 * @param obj The object to check.
 * @return true if the object is a json_array, false otherwise.
 */
inline bool is_array(const std::shared_ptr<json_object>& obj) {
    return std::dynamic_pointer_cast<json_array>(obj) != nullptr;
}

/**
 * @brief Checks if a json_object is an object (not array, string, number, or boolean).
 * @param obj The object to check.
 * @return true if the object is a json_object (and not a derived type), false otherwise.
 */
inline bool is_object(const std::shared_ptr<json_object>& obj) {
    if (!obj)
        return false;
    return !is_string(obj) && !is_number(obj) && !is_boolean(obj) && !is_array(obj);
}

/**
 * @brief Checks if a json_object is null.
 * @param obj The object to check.
 * @return true if the object is nullptr, false otherwise.
 */
inline bool is_null(const std::shared_ptr<json_object>& obj) {
    return obj == nullptr;
}

}  // namespace cppress::json

namespace cppress::json::maker {

/**
 * @brief Creates a JSON string object.
 * @param value The string value.
 * @return A shared pointer to a json_string.
 */
inline std::shared_ptr<cppress::json::json_object> make_string(const std::string& value) {
    return std::make_shared<cppress::json::json_string>(value);
}

/**
 * @brief Creates a JSON number object from a double.
 * @param value The numeric value.
 * @return A shared pointer to a json_number.
 */
inline std::shared_ptr<cppress::json::json_object> make_number(double value) {
    return std::make_shared<cppress::json::json_number>(value);
}

/**
 * @brief Creates a JSON number object from an int.
 * @param value The integer value.
 * @return A shared pointer to a json_number.
 */
inline std::shared_ptr<cppress::json::json_object> make_number(int value) {
    return std::make_shared<cppress::json::json_number>(value);
}

/**
 * @brief Creates a JSON number object from a long.
 * @param value The long value.
 * @return A shared pointer to a json_number.
 */
inline std::shared_ptr<cppress::json::json_object> make_number(long value) {
    return std::make_shared<cppress::json::json_number>(value);
}

/**
 * @brief Creates a JSON number object from a long long.
 * @param value The long long value.
 * @return A shared pointer to a json_number.
 */
inline std::shared_ptr<cppress::json::json_object> make_number(long long value) {
    return std::make_shared<cppress::json::json_number>(value);
}

/**
 * @brief Creates a JSON boolean object.
 * @param value The boolean value.
 * @return A shared pointer to a json_boolean.
 */
inline std::shared_ptr<cppress::json::json_object> make_boolean(bool value) {
    return std::make_shared<cppress::json::json_boolean>(value);
}

/**
 * @brief Creates an empty JSON array object.
 * @return A shared pointer to a json_array.
 */
inline std::shared_ptr<cppress::json::json_array> make_array() {
    return std::make_shared<cppress::json::json_array>();
}

/**
 * @brief Creates a JSON array object from a vector.
 * @param elements The initial elements.
 * @return A shared pointer to a json_array.
 */
inline std::shared_ptr<cppress::json::json_array> make_array(
    const std::vector<std::shared_ptr<cppress::json::json_object>>& elements) {
    return std::make_shared<cppress::json::json_array>(elements);
}

/**
 * @brief Creates an empty JSON object.
 * @return A shared pointer to a json_object.
 */
inline std::shared_ptr<cppress::json::json_object> make_object() {
    return std::make_shared<cppress::json::json_object>();
}

/**
 * @brief Creates a JSON null value.
 * @return A nullptr representing JSON null.
 */
inline std::shared_ptr<cppress::json::json_object> make_null() {
    return nullptr;
}

}  // namespace cppress::json::maker

namespace cppress::json::getter {

/**
 * @brief Gets the boolean value from a json_object.
 * @param obj The JSON object.
 * @return The boolean value.
 * @throws std::runtime_error if the object is not a json_boolean.
 */
inline bool get_boolean(const std::shared_ptr<cppress::json::json_object>& obj) {
    if (auto boolean = std::dynamic_pointer_cast<cppress::json::json_boolean>(obj)) {
        return boolean->value;
    }
    throw std::runtime_error("Not a boolean");
}

/**
 * @brief Safely gets the boolean value from a json_object.
 * @param obj The JSON object.
 * @return An optional containing the value if successful, or empty if not a boolean.
 */
inline std::optional<bool> try_get_boolean(const std::shared_ptr<cppress::json::json_object>& obj) {
    if (auto boolean = std::dynamic_pointer_cast<cppress::json::json_boolean>(obj)) {
        return boolean->value;
    }
    return std::nullopt;
}

/**
 * @brief Gets the numeric value from a json_object.
 * @param obj The JSON object.
 * @return The numeric value.
 * @throws std::runtime_error if the object is not a json_number.
 */
inline double get_number(const std::shared_ptr<cppress::json::json_object>& obj) {
    if (auto number = std::dynamic_pointer_cast<cppress::json::json_number>(obj)) {
        return number->value;
    }
    throw std::runtime_error("Not a number");
}

/**
 * @brief Safely gets the numeric value from a json_object.
 * @param obj The JSON object.
 * @return An optional containing the value if successful, or empty if not a number.
 */
inline std::optional<double> try_get_number(
    const std::shared_ptr<cppress::json::json_object>& obj) {
    if (auto number = std::dynamic_pointer_cast<cppress::json::json_number>(obj)) {
        return number->value;
    }
    return std::nullopt;
}

/**
 * @brief Gets the integer value from a json_object.
 * @param obj The JSON object.
 * @return The integer value.
 * @throws std::runtime_error if the object is not a json_number.
 */
inline int get_int(const std::shared_ptr<cppress::json::json_object>& obj) {
    if (auto number = std::dynamic_pointer_cast<cppress::json::json_number>(obj)) {
        return number->to_int();
    }
    throw std::runtime_error("Not a number");
}

/**
 * @brief Safely gets the integer value from a json_object.
 * @param obj The JSON object.
 * @return An optional containing the value if successful, or empty if not a number.
 */
inline std::optional<int> try_get_int(const std::shared_ptr<cppress::json::json_object>& obj) {
    if (auto number = std::dynamic_pointer_cast<cppress::json::json_number>(obj)) {
        return number->to_int();
    }
    return std::nullopt;
}

/**
 * @brief Gets the string value from a json_object.
 * @param obj The JSON object.
 * @return The string value.
 * @throws std::runtime_error if the object is not a json_string.
 */
inline std::string get_string(const std::shared_ptr<cppress::json::json_object>& obj) {
    if (auto str = std::dynamic_pointer_cast<cppress::json::json_string>(obj)) {
        return str->value;
    }
    throw std::runtime_error("Not a string");
}

/**
 * @brief Safely gets the string value from a json_object.
 * @param obj The JSON object.
 * @return An optional containing the value if successful, or empty if not a string.
 */
inline std::optional<std::string> try_get_string(
    const std::shared_ptr<cppress::json::json_object>& obj) {
    if (auto str = std::dynamic_pointer_cast<cppress::json::json_string>(obj)) {
        return str->value;
    }
    return std::nullopt;
}

/**
 * @brief Gets the array elements from a json_object.
 * @param obj The JSON object.
 * @return The vector of array elements.
 * @throws std::runtime_error if the object is not a json_array.
 */
inline std::vector<std::shared_ptr<cppress::json::json_object>> get_array(
    const std::shared_ptr<cppress::json::json_object>& obj) {
    if (auto array = std::dynamic_pointer_cast<cppress::json::json_array>(obj)) {
        return array->elements;
    }
    throw std::runtime_error("Not an array");
}

/**
 * @brief Safely gets the array elements from a json_object.
 * @param obj The JSON object.
 * @return An optional containing the elements if successful, or empty if not an array.
 */
inline std::optional<std::vector<std::shared_ptr<cppress::json::json_object>>> try_get_array(
    const std::shared_ptr<cppress::json::json_object>& obj) {
    if (auto array = std::dynamic_pointer_cast<cppress::json::json_array>(obj)) {
        return array->elements;
    }
    return std::nullopt;
}

/**
 * @brief Casts a json_object to json_array.
 * @param obj The JSON object.
 * @return A shared pointer to json_array.
 * @throws std::runtime_error if the object is not a json_array.
 */
inline std::shared_ptr<cppress::json::json_array> as_array(
    const std::shared_ptr<cppress::json::json_object>& obj) {
    if (auto array = std::dynamic_pointer_cast<cppress::json::json_array>(obj)) {
        return array;
    }
    throw std::runtime_error("Not an array");
}

/**
 * @brief Safely casts a json_object to json_array.
 * @param obj The JSON object.
 * @return A shared pointer to json_array, or nullptr if not an array.
 */
inline std::shared_ptr<cppress::json::json_array> try_as_array(
    const std::shared_ptr<cppress::json::json_object>& obj) {
    return std::dynamic_pointer_cast<cppress::json::json_array>(obj);
}

/**
 * @brief Casts a json_object to json_string.
 * @param obj The JSON object.
 * @return A shared pointer to json_string.
 * @throws std::runtime_error if the object is not a json_string.
 */
inline std::shared_ptr<cppress::json::json_string> as_string(
    const std::shared_ptr<cppress::json::json_object>& obj) {
    if (auto str = std::dynamic_pointer_cast<cppress::json::json_string>(obj)) {
        return str;
    }
    throw std::runtime_error("Not a string");
}

/**
 * @brief Safely casts a json_object to json_string.
 * @param obj The JSON object.
 * @return A shared pointer to json_string, or nullptr if not a string.
 */
inline std::shared_ptr<cppress::json::json_string> try_as_string(
    const std::shared_ptr<cppress::json::json_object>& obj) {
    return std::dynamic_pointer_cast<cppress::json::json_string>(obj);
}

/**
 * @brief Casts a json_object to json_number.
 * @param obj The JSON object.
 * @return A shared pointer to json_number.
 * @throws std::runtime_error if the object is not a json_number.
 */
inline std::shared_ptr<cppress::json::json_number> as_number(
    const std::shared_ptr<cppress::json::json_object>& obj) {
    if (auto num = std::dynamic_pointer_cast<cppress::json::json_number>(obj)) {
        return num;
    }
    throw std::runtime_error("Not a number");
}

/**
 * @brief Safely casts a json_object to json_number.
 * @param obj The JSON object.
 * @return A shared pointer to json_number, or nullptr if not a number.
 */
inline std::shared_ptr<cppress::json::json_number> try_as_number(
    const std::shared_ptr<cppress::json::json_object>& obj) {
    return std::dynamic_pointer_cast<cppress::json::json_number>(obj);
}

/**
 * @brief Casts a json_object to json_boolean.
 * @param obj The JSON object.
 * @return A shared pointer to json_boolean.
 * @throws std::runtime_error if the object is not a json_boolean.
 */
inline std::shared_ptr<cppress::json::json_boolean> as_boolean(
    const std::shared_ptr<cppress::json::json_object>& obj) {
    if (auto boolean = std::dynamic_pointer_cast<cppress::json::json_boolean>(obj)) {
        return boolean;
    }
    throw std::runtime_error("Not a boolean");
}

/**
 * @brief Safely casts a json_object to json_boolean.
 * @param obj The JSON object.
 * @return A shared pointer to json_boolean, or nullptr if not a boolean.
 */
inline std::shared_ptr<cppress::json::json_boolean> try_as_boolean(
    const std::shared_ptr<cppress::json::json_object>& obj) {
    return std::dynamic_pointer_cast<cppress::json::json_boolean>(obj);
}

}  // namespace cppress::json::getter