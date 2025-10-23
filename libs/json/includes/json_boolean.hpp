#pragma once

#include <algorithm>
#include <stdexcept>

#include "json_object.hpp"

namespace cppress::json {

/**
 * @class json_boolean
 * @brief Represents a JSON boolean value with type-safe operations.
 *
 * This class provides a JSON boolean implementation with methods for
 * boolean operations and type conversions.
 *
 * @note Inherits from json_object but represents a primitive boolean value.
 */
class json_boolean : public json_object {
public:
    // STL-like type alias
    using value_type = bool;

    /// The actual boolean value
    bool value;

    // Constructors and destructor
    /**
     * @brief Default constructor. Initializes value to false.
     */
    json_boolean() = default;

    /**
     * @brief Constructs a JSON boolean from a bool.
     * @param value The boolean value.
     */
    json_boolean(bool value) : value(value) {}

    /**
     * @brief Default destructor.
     */
    ~json_boolean() = default;

    // Override base class methods
    /**
     * @brief Throws an exception as booleans don't contain objects.
     * @param key Unused parameter.
     * @return Never returns.
     * @throws std::runtime_error Always throws.
     */
    virtual std::shared_ptr<json_object> get(
        [[maybe_unused]] const std::string& key) const override {
        throw std::runtime_error("json_boolean does not contain objects");
    }

    /**
     * @brief Parses a string and sets this boolean's value.
     * @param temp The string representation ("true" or "false", case-insensitive).
     * @return true if parsing succeeded, false otherwise.
     */
    bool set_json_data(const std::string& temp) override {
        auto jsonString = temp;
        std::transform(jsonString.begin(), jsonString.end(), jsonString.begin(), ::tolower);
        if (jsonString == "true") {
            value = true;
            return true;
        } else if (jsonString == "false") {
            value = false;
            return true;
        }
        return false;
    }

    /**
     * @brief Converts this boolean to a JSON string.
     * @return "true" or "false".
     */
    std::string stringify() const override { return value ? "true" : "false"; }

    // Type-safe conversion methods
    /**
     * @brief Gets the boolean value.
     * @return The boolean value.
     */
    bool get_value() const noexcept { return value; }

    /**
     * @brief Sets the boolean value.
     * @param new_value The new boolean value.
     */
    void set_value(bool new_value) noexcept { value = new_value; }

    /**
     * @brief Converts to integer (0 or 1).
     * @return 1 if true, 0 if false.
     */
    int to_int() const noexcept { return value ? 1 : 0; }

    /**
     * @brief Converts to string.
     * @return "true" or "false".
     */
    std::string to_string() const { return stringify(); }

    // Conversion operators
    /**
     * @brief Implicit conversion to bool.
     * @return The boolean value.
     */
    operator bool() const noexcept { return value; }

    /**
     * @brief Explicit conversion to int.
     * @return 1 if true, 0 if false.
     */
    explicit operator int() const noexcept { return to_int(); }

    // Logical operators
    /**
     * @brief Logical AND assignment operator.
     * @param rhs The right-hand side value.
     * @return Reference to this object.
     */
    json_boolean& operator&=(bool rhs) noexcept {
        value = value && rhs;
        return *this;
    }

    /**
     * @brief Logical OR assignment operator.
     * @param rhs The right-hand side value.
     * @return Reference to this object.
     */
    json_boolean& operator|=(bool rhs) noexcept {
        value = value || rhs;
        return *this;
    }

    /**
     * @brief Logical XOR assignment operator.
     * @param rhs The right-hand side value.
     * @return Reference to this object.
     */
    json_boolean& operator^=(bool rhs) noexcept {
        value = value != rhs;
        return *this;
    }

    /**
     * @brief Logical NOT operator.
     * @return Negated value.
     */
    bool operator!() const noexcept { return !value; }

    /**
     * @brief Assignment operator from bool.
     * @param rhs The boolean value to assign.
     * @return Reference to this object.
     */
    json_boolean& operator=(bool rhs) noexcept {
        value = rhs;
        return *this;
    }

    // Comparison operators
    /**
     * @brief Equality comparison with bool.
     * @param rhs The value to compare with.
     * @return true if equal, false otherwise.
     */
    bool operator==(bool rhs) const noexcept { return value == rhs; }

    /**
     * @brief Inequality comparison with bool.
     * @param rhs The value to compare with.
     * @return true if not equal, false otherwise.
     */
    bool operator!=(bool rhs) const noexcept { return value != rhs; }
};

}  // namespace cppress::json
