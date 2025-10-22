#pragma once

#include <string>

#include "json_object.hpp"

namespace cppress::json {

/**
 * @class json_string
 * @brief Represents a JSON string value with an STL-like interface.
 *
 * This class provides a JSON string implementation with methods similar to
 * std::string, offering familiar STL-style operations for working with
 * JSON string data.
 *
 * @note Inherits from json_object but represents a primitive string value.
 */
class json_string : public json_object {
public:
    // STL-like type aliases
    using value_type = std::string;
    using size_type = std::size_t;
    using iterator = std::string::iterator;
    using const_iterator = std::string::const_iterator;

    /// The actual string value
    std::string value;

    // Constructors and destructor
    /**
     * @brief Default constructor. Creates an empty string.
     */
    json_string() = default;

    /**
     * @brief Constructs a JSON string from a std::string.
     * @param value The string value.
     */
    json_string(const std::string& value) : value(value) {}

    /**
     * @brief Default destructor.
     */
    ~json_string() = default;

    // Override base class methods
    /**
     * @brief Throws an exception as strings don't contain objects.
     * @param key Unused parameter.
     * @return Never returns.
     * @throws std::runtime_error Always throws.
     */
    std::shared_ptr<json_object> get([[maybe_unused]] const std::string& key) const override {
        throw std::runtime_error("json_string does not contain objects");
    }

    /**
     * @brief Sets the string value from a JSON string.
     * @param jsonString The string value to set.
     * @return true (always succeeds).
     */
    bool set_json_data(const std::string& jsonString) override {
        value = jsonString;
        return true;
    }

    /**
     * @brief Converts this string to a JSON string (with quotes and escaping).
     * @return A JSON string representation with proper escaping.
     */
    std::string stringify() const override {
        // Handle escape characters properly
        std::string escaped = value;
        size_type pos = 0;

        // Escape backslashes first (must be done before other escapes)
        while ((pos = escaped.find("\\", pos)) != std::string::npos) {
            escaped.replace(pos, 1, "\\\\");
            pos += 2;  // Skip the newly inserted characters
        }

        // Escape double quotes
        pos = 0;
        while ((pos = escaped.find("\"", pos)) != std::string::npos) {
            escaped.replace(pos, 1, "\\\"");
            pos += 2;  // Skip the newly inserted characters
        }

        // Escape other common characters
        pos = 0;
        while ((pos = escaped.find("\n", pos)) != std::string::npos) {
            escaped.replace(pos, 1, "\\n");
            pos += 2;
        }

        pos = 0;
        while ((pos = escaped.find("\r", pos)) != std::string::npos) {
            escaped.replace(pos, 1, "\\r");
            pos += 2;
        }

        pos = 0;
        while ((pos = escaped.find("\t", pos)) != std::string::npos) {
            escaped.replace(pos, 1, "\\t");
            pos += 2;
        }

        pos = 0;
        while ((pos = escaped.find("\b", pos)) != std::string::npos) {
            escaped.replace(pos, 1, "\\b");
            pos += 2;
        }

        pos = 0;
        while ((pos = escaped.find("\f", pos)) != std::string::npos) {
            escaped.replace(pos, 1, "\\f");
            pos += 2;
        }

        return "\"" + escaped + "\"";
    }

    // STL-like string methods
    /**
     * @brief Returns the length of the string.
     * @return The number of characters in the string.
     */
    size_type size() const noexcept { return value.size(); }

    /**
     * @brief Returns the length of the string (alias for size).
     * @return The number of characters in the string.
     */
    size_type length() const noexcept { return value.length(); }

    /**
     * @brief Checks if the string is empty.
     * @return true if the string has no characters, false otherwise.
     */
    bool empty() const noexcept { return value.empty(); }

    /**
     * @brief Returns a C-style string pointer.
     * @return A pointer to the null-terminated character array.
     */
    const char* c_str() const noexcept { return value.c_str(); }

    /**
     * @brief Returns a pointer to the string data.
     * @return A pointer to the character array.
     */
    const char* data() const noexcept { return value.data(); }

    /**
     * @brief Clears the string content.
     */
    void clear() noexcept { value.clear(); }

    /**
     * @brief Appends a string to the end.
     * @param str The string to append.
     */
    void append(const std::string& str) { value.append(str); }

    /**
     * @brief Extracts a substring.
     * @param pos Starting position.
     * @param len Length of the substring (default: entire remaining string).
     * @return A new string containing the substring.
     */
    std::string substr(size_type pos = 0, size_type len = std::string::npos) const {
        return value.substr(pos, len);
    }

    /**
     * @brief Finds a substring.
     * @param str The substring to find.
     * @param pos Starting position for the search.
     * @return Position of the first occurrence, or std::string::npos if not found.
     */
    size_type find(const std::string& str, size_type pos = 0) const noexcept {
        return value.find(str, pos);
    }

    // Iterator support
    /**
     * @brief Returns an iterator to the beginning.
     * @return Iterator to the first character.
     */
    iterator begin() noexcept { return value.begin(); }

    /**
     * @brief Returns a const iterator to the beginning.
     * @return Const iterator to the first character.
     */
    const_iterator begin() const noexcept { return value.begin(); }

    /**
     * @brief Returns a const iterator to the beginning.
     * @return Const iterator to the first character.
     */
    const_iterator cbegin() const noexcept { return value.cbegin(); }

    /**
     * @brief Returns an iterator to the end.
     * @return Iterator to one past the last character.
     */
    iterator end() noexcept { return value.end(); }

    /**
     * @brief Returns a const iterator to the end.
     * @return Const iterator to one past the last character.
     */
    const_iterator end() const noexcept { return value.end(); }

    /**
     * @brief Returns a const iterator to the end.
     * @return Const iterator to one past the last character.
     */
    const_iterator cend() const noexcept { return value.cend(); }

    // Conversion operators
    /**
     * @brief Implicit conversion to std::string.
     * @return The string value.
     */
    operator std::string() const { return value; }

    /**
     * @brief Explicit conversion to std::string.
     * @return The string value.
     */
    std::string str() const { return value; }

    // Operator overloads
    /**
     * @brief Appends a string using the += operator.
     * @param str The string to append.
     * @return Reference to this object.
     */
    json_string& operator+=(const std::string& str) {
        value += str;
        return *this;
    }

    /**
     * @brief Accesses a character by index without bounds checking.
     * @param index The index of the character.
     * @return Reference to the character.
     */
    char& operator[](size_type index) { return value[index]; }

    /**
     * @brief Accesses a character by index without bounds checking (const version).
     * @param index The index of the character.
     * @return The character.
     */
    const char& operator[](size_type index) const { return value[index]; }
};

}  // namespace cppress