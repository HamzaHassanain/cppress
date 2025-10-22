#pragma once

#include <memory>
#include <stdexcept>
#include <vector>

#include "json_object.hpp"
#include "parser.hpp"

namespace cppress::json {

/**
 * @class json_array
 * @brief Represents a JSON array with an STL-like interface.
 *
 * This class provides a JSON array implementation with methods similar to
 * std::vector, offering familiar STL-style operations for working with
 * JSON array data.
 *
 * @note Inherits from json_object but provides array-specific functionality.
 */
class json_array : public json_object {
public:
    // STL-like type aliases
    using value_type = std::shared_ptr<json_object>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator = std::vector<value_type>::iterator;
    using const_iterator = std::vector<value_type>::const_iterator;
    using reverse_iterator = std::vector<value_type>::reverse_iterator;
    using const_reverse_iterator = std::vector<value_type>::const_reverse_iterator;

    /// Internal storage for array elements
    std::vector<std::shared_ptr<json_object>> elements;

    // Constructors and destructor
    /**
     * @brief Default constructor. Creates an empty JSON array.
     */
    json_array() = default;

    /**
     * @brief Constructs a JSON array from a vector of elements.
     * @param elements The initial elements for the array.
     */
    json_array(const std::vector<std::shared_ptr<json_object>>& elements) : elements(elements) {}

    /**
     * @brief Default destructor.
     */
    ~json_array() = default;

    // Override base class methods
    /**
     * @brief Gets an element by string key (interprets as index).
     * @param key String representation of the index.
     * @return A shared pointer to the element.
     * @throws std::runtime_error if key is not numeric.
     * @throws std::out_of_range if index is out of bounds.
     */
    std::shared_ptr<json_object> get([[maybe_unused]] const std::string& key) const override {
        try {
            int idx = std::stoi(key);

            if (idx < 0 || idx >= static_cast<int>(elements.size())) {
                throw std::out_of_range("Invalid array index");
            }
            return elements[idx];
        } catch (const std::invalid_argument&) {
            throw std::runtime_error(
                "json_array does not support key-based access with non-numeric keys");
        } catch (const std::out_of_range&) {
            throw std::runtime_error("json_array index out of range");
        }
    }

    /**
     * @brief Parses a JSON array string and sets this array's data.
     * @param jsonString The JSON array string to parse.
     * @return true if parsing succeeded, false otherwise.
     */
    bool set_json_data([[maybe_unused]] const std::string& jsonString) override {
        auto obj = json_value(jsonString);
        if (!obj) {
            return false;
        }
        if (auto arr = std::dynamic_pointer_cast<json_array>(obj)) {
            elements = arr->elements;
        } else {
            return false;
        }
        return true;
    }

    /**
     * @brief Converts this array to a JSON string.
     * @return A JSON string representation of this array.
     */
    std::string stringify() const override {
        std::string result = "[";
        for (const auto& element : elements) {
            if (element) {
                result += element->stringify() + ",";
            } else {
                result += "null,";
            }
        }
        if (result.back() == ',') {
            result.pop_back();  // Remove trailing comma
        }
        result += "]";
        return result;
    }

    // STL-like array-specific methods
    /**
     * @brief Adds an element to the end of the array.
     * @param value The element to add.
     */
    void push_back(std::shared_ptr<json_object> value) { elements.push_back(value); }

    /**
     * @brief Adds an element to the end (alias for push_back).
     * @param value The element to add.
     * @note This maintains compatibility with the existing interface.
     */
    void insert(std::shared_ptr<json_object> value) { elements.push_back(value); }

    /**
     * @brief Removes the last element from the array.
     * @throws std::out_of_range if the array is empty.
     */
    void pop_back() {
        if (elements.empty()) {
            throw std::out_of_range("Cannot pop from empty array");
        }
        elements.pop_back();
    }

    /**
     * @brief Returns the number of elements in the array.
     * @return The number of elements.
     */
    size_type size() const noexcept { return elements.size(); }

    /**
     * @brief Checks if the array is empty.
     * @return true if the array contains no elements, false otherwise.
     */
    bool empty() const noexcept { return elements.empty(); }

    /**
     * @brief Accesses an element by index with bounds checking.
     * @param index The index of the element to access.
     * @return A reference to the element.
     * @throws std::out_of_range if index is out of bounds.
     */
    reference at(size_type index) {
        if (index >= elements.size()) {
            throw std::out_of_range("Array index out of bounds");
        }
        return elements[index];
    }

    /**
     * @brief Accesses an element by index with bounds checking (const version).
     * @param index The index of the element to access.
     * @return A const reference to the element.
     * @throws std::out_of_range if index is out of bounds.
     */
    const_reference at(size_type index) const {
        if (index >= elements.size()) {
            throw std::out_of_range("Array index out of bounds");
        }
        return elements[index];
    }

    /**
     * @brief Accesses the first element.
     * @return A reference to the first element.
     * @throws std::out_of_range if the array is empty.
     */
    reference front() {
        if (elements.empty()) {
            throw std::out_of_range("Array is empty");
        }
        return elements.front();
    }

    /**
     * @brief Accesses the first element (const version).
     * @return A const reference to the first element.
     * @throws std::out_of_range if the array is empty.
     */
    const_reference front() const {
        if (elements.empty()) {
            throw std::out_of_range("Array is empty");
        }
        return elements.front();
    }

    /**
     * @brief Accesses the last element.
     * @return A reference to the last element.
     * @throws std::out_of_range if the array is empty.
     */
    reference back() {
        if (elements.empty()) {
            throw std::out_of_range("Array is empty");
        }
        return elements.back();
    }

    /**
     * @brief Accesses the last element (const version).
     * @return A const reference to the last element.
     * @throws std::out_of_range if the array is empty.
     */
    const_reference back() const {
        if (elements.empty()) {
            throw std::out_of_range("Array is empty");
        }
        return elements.back();
    }

    /**
     * @brief Removes all elements from the array.
     */
    void clear() noexcept { elements.clear(); }

    /**
     * @brief Resizes the array.
     * @param count New size of the array.
     */
    void resize(size_type count) { elements.resize(count); }

    /**
     * @brief Reserves capacity for the array.
     * @param capacity Minimum capacity to reserve.
     */
    void reserve(size_type capacity) { elements.reserve(capacity); }

    // Iterator support
    /**
     * @brief Returns an iterator to the beginning.
     * @return Iterator to the first element.
     */
    iterator begin() noexcept { return elements.begin(); }

    /**
     * @brief Returns a const iterator to the beginning.
     * @return Const iterator to the first element.
     */
    const_iterator begin() const noexcept { return elements.begin(); }

    /**
     * @brief Returns a const iterator to the beginning.
     * @return Const iterator to the first element.
     */
    const_iterator cbegin() const noexcept { return elements.cbegin(); }

    /**
     * @brief Returns an iterator to the end.
     * @return Iterator to one past the last element.
     */
    iterator end() noexcept { return elements.end(); }

    /**
     * @brief Returns a const iterator to the end.
     * @return Const iterator to one past the last element.
     */
    const_iterator end() const noexcept { return elements.end(); }

    /**
     * @brief Returns a const iterator to the end.
     * @return Const iterator to one past the last element.
     */
    const_iterator cend() const noexcept { return elements.cend(); }

    /**
     * @brief Returns a reverse iterator to the beginning.
     * @return Reverse iterator to the first element of reversed array.
     */
    reverse_iterator rbegin() noexcept { return elements.rbegin(); }

    /**
     * @brief Returns a const reverse iterator to the beginning.
     * @return Const reverse iterator to the first element of reversed array.
     */
    const_reverse_iterator rbegin() const noexcept { return elements.rbegin(); }

    /**
     * @brief Returns a const reverse iterator to the beginning.
     * @return Const reverse iterator to the first element of reversed array.
     */
    const_reverse_iterator crbegin() const noexcept { return elements.crbegin(); }

    /**
     * @brief Returns a reverse iterator to the end.
     * @return Reverse iterator to one past the last element of reversed array.
     */
    reverse_iterator rend() noexcept { return elements.rend(); }

    /**
     * @brief Returns a const reverse iterator to the end.
     * @return Const reverse iterator to one past the last element of reversed array.
     */
    const_reverse_iterator rend() const noexcept { return elements.rend(); }

    /**
     * @brief Returns a const reverse iterator to the end.
     * @return Const reverse iterator to one past the last element of reversed array.
     */
    const_reverse_iterator crend() const noexcept { return elements.crend(); }

    // Operator overloads
    /**
     * @brief Accesses an element by index without bounds checking.
     * @param index The index of the element.
     * @return A reference to the element.
     */
    reference operator[](size_type index) { return elements[index]; }

    /**
     * @brief Accesses an element by index without bounds checking (const version).
     * @param index The index of the element.
     * @return A const reference to the element.
     */
    const_reference operator[](size_type index) const { return elements[index]; }
};

}  // namespace cppress::json