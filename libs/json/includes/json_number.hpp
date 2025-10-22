#pragma once

#include <cmath>

#include "json_object.hpp"

namespace cppress::json {

/**
 * @class json_number
 * @brief Represents a JSON numeric value with type-safe operations.
 *
 * This class provides a JSON number implementation with methods for
 * numeric operations and type conversions. Internally stores numbers
 * as double precision floating point values.
 *
 * @note Inherits from json_object but represents a primitive numeric value.
 */
class json_number : public json_object {
public:
    // STL-like type alias
    using value_type = double;

    /// The actual numeric value
    double value;

    // Constructors and destructor
    /**
     * @brief Default constructor. Initializes value to 0.
     */
    json_number() = default;

    /**
     * @brief Constructs a JSON number from a double.
     * @param value The numeric value.
     */
    json_number(double value) : value(value) {}

    /**
     * @brief Constructs a JSON number from an integer.
     * @param value The integer value.
     */
    json_number(int value) : value(static_cast<double>(value)) {}

    /**
     * @brief Constructs a JSON number from a long.
     * @param value The long value.
     */
    json_number(long value) : value(static_cast<double>(value)) {}

    /**
     * @brief Constructs a JSON number from a long long.
     * @param value The long long value.
     */
    json_number(long long value) : value(static_cast<double>(value)) {}

    /**
     * @brief Constructs a JSON number from a float.
     * @param value The float value.
     */
    json_number(float value) : value(static_cast<double>(value)) {}

    /**
     * @brief Default destructor.
     */
    ~json_number() = default;

    // Override base class methods
    /**
     * @brief Throws an exception as numbers don't contain objects.
     * @param key Unused parameter.
     * @return Never returns.
     * @throws std::runtime_error Always throws.
     */
    virtual std::shared_ptr<json_object> get(
        [[maybe_unused]] const std::string& key) const override {
        throw std::runtime_error("json_number does not contain objects");
    }

    /**
     * @brief Parses a string and sets this number's value.
     * @param jsonString The string representation of the number.
     * @return true if parsing succeeded, false otherwise.
     */
    bool set_json_data(const std::string& jsonString) override {
        if (jsonString.empty()) {
            return false;
        }

        try {
            size_t pos = 0;
            double parsed_value = std::stod(jsonString, &pos);

            // Check if the entire string was consumed (no trailing characters)
            if (pos != jsonString.length()) {
                return false;
            }

            value = parsed_value;
            return true;
        } catch (...) {
            return false;
        }
    }

    /**
     * @brief Converts this number to a JSON string.
     * @return A string representation of the number.
     * @note Integers are formatted without decimal points.
     */
    std::string stringify() const override {
        if ((long long)value == value)
            return std::to_string((long long)value);
        return std::to_string(value);
    }

    // Type-safe conversion methods
    /**
     * @brief Converts to int.
     * @return The value as an int.
     */
    int to_int() const noexcept { return static_cast<int>(value); }

    /**
     * @brief Converts to long.
     * @return The value as a long.
     */
    long to_long() const noexcept { return static_cast<long>(value); }

    /**
     * @brief Converts to long long.
     * @return The value as a long long.
     */
    long long to_long_long() const noexcept { return static_cast<long long>(value); }

    /**
     * @brief Converts to float.
     * @return The value as a float.
     */
    float to_float() const noexcept { return static_cast<float>(value); }

    /**
     * @brief Converts to double.
     * @return The value as a double.
     */
    double to_double() const noexcept { return value; }

    /**
     * @brief Checks if the number is an integer.
     * @return true if the value has no fractional part, false otherwise.
     */
    bool is_integer() const noexcept { return value == static_cast<long long>(value); }

    /**
     * @brief Checks if the number is finite (not infinity or NaN).
     * @return true if the value is finite, false otherwise.
     */
    bool is_finite() const noexcept { return std::isfinite(value); }

    /**
     * @brief Checks if the number is NaN.
     * @return true if the value is NaN, false otherwise.
     */
    bool is_nan() const noexcept { return std::isnan(value); }

    /**
     * @brief Checks if the number is infinity.
     * @return true if the value is positive or negative infinity, false otherwise.
     */
    bool is_infinity() const noexcept { return std::isinf(value); }

    // Conversion operators
    /**
     * @brief Implicit conversion to double.
     * @return The numeric value.
     */
    operator double() const noexcept { return value; }

    /**
     * @brief Explicit conversion to int.
     * @return The value as an int.
     */
    explicit operator int() const noexcept { return to_int(); }

    /**
     * @brief Explicit conversion to long.
     * @return The value as a long.
     */
    explicit operator long() const noexcept { return to_long(); }

    /**
     * @brief Explicit conversion to long long.
     * @return The value as a long long.
     */
    explicit operator long long() const noexcept { return to_long_long(); }

    /**
     * @brief Explicit conversion to float.
     * @return The value as a float.
     */
    explicit operator float() const noexcept { return to_float(); }

    // Arithmetic operators
    /**
     * @brief Addition assignment operator.
     * @param rhs The value to add.
     * @return Reference to this object.
     */
    json_number& operator+=(double rhs) noexcept {
        value += rhs;
        return *this;
    }

    /**
     * @brief Subtraction assignment operator.
     * @param rhs The value to subtract.
     * @return Reference to this object.
     */
    json_number& operator-=(double rhs) noexcept {
        value -= rhs;
        return *this;
    }

    /**
     * @brief Multiplication assignment operator.
     * @param rhs The value to multiply by.
     * @return Reference to this object.
     */
    json_number& operator*=(double rhs) noexcept {
        value *= rhs;
        return *this;
    }

    /**
     * @brief Division assignment operator.
     * @param rhs The value to divide by.
     * @return Reference to this object.
     */
    json_number& operator/=(double rhs) noexcept {
        value /= rhs;
        return *this;
    }

    /**
     * @brief Prefix increment operator.
     * @return Reference to this object.
     */
    json_number& operator++() noexcept {
        ++value;
        return *this;
    }

    /**
     * @brief Postfix increment operator.
     * @return Copy of the object before increment.
     */
    json_number operator++(int) noexcept {
        json_number temp = *this;
        ++value;
        return temp;
    }

    /**
     * @brief Prefix decrement operator.
     * @return Reference to this object.
     */
    json_number& operator--() noexcept {
        --value;
        return *this;
    }

    /**
     * @brief Postfix decrement operator.
     * @return Copy of the object before decrement.
     */
    json_number operator--(int) noexcept {
        json_number temp = *this;
        --value;
        return temp;
    }
};

}  // namespace cppress
