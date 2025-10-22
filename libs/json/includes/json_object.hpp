#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace cppress::json {

/**
 * @class json_object
 * @brief Represents a JSON object with an STL-like interface.
 *
 * This class provides a JSON object implementation with methods similar to
 * std::unordered_map, offering familiar STL-style operations for working with
 * JSON data. It supports nested objects, arrays, and all JSON value types.
 *
 * @note This is the base class for all JSON value types.
 */
class json_object {
protected:
    /// Internal storage for key-value pairs
    std::unordered_map<std::string, std::shared_ptr<json_object>> data;

public:
    // STL-like type aliases
    using key_type = std::string;
    using mapped_type = std::shared_ptr<json_object>;
    using value_type = std::pair<const key_type, mapped_type>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator = std::unordered_map<key_type, mapped_type>::iterator;
    using const_iterator = std::unordered_map<key_type, mapped_type>::const_iterator;

    // Constructors and destructor
    /**
     * @brief Default constructor. Creates an empty JSON object.
     */
    json_object();

    /**
     * @brief Constructs a JSON object from an existing map.
     * @param initial_data The initial key-value pairs for the object.
     */
    json_object(const std::unordered_map<std::string, std::shared_ptr<json_object>>& initial_data);

    /**
     * @brief Virtual destructor for proper inheritance.
     */
    virtual ~json_object();

    // Data manipulation
    /**
     * @brief Parses a JSON string and sets this object's data.
     * @param jsonString The JSON string to parse.
     * @return true if parsing succeeded, false otherwise.
     * @throws std::runtime_error if parsing fails.
     */
    virtual bool set_json_data(const std::string& jsonString);

    /**
     * @brief Inserts or updates a key-value pair.
     * @param key The key to insert or update.
     * @param value The value to associate with the key.
     * @note If the key already exists, its value is overwritten.
     */
    virtual void insert(const std::string& key, std::shared_ptr<json_object> value);

    /**
     * @brief Removes an element by key.
     * @param key The key of the element to remove.
     * @return The number of elements removed (0 or 1).
     */
    virtual size_type erase(const std::string& key);

    /**
     * @brief Retrieves a value by key.
     * @param key The key to look up.
     * @return A shared pointer to the value, or nullptr if not found.
     */
    virtual std::shared_ptr<json_object> get(const std::string& key) const;

    /**
     * @brief Accesses an element by key with bounds checking.
     * @param key The key of the element to access.
     * @return A shared pointer to the value.
     * @throws std::out_of_range if the key does not exist.
     */
    virtual std::shared_ptr<json_object> at(const std::string& key) const;

    /**
     * @brief Converts this object to a JSON string.
     * @return A JSON string representation of this object.
     */
    virtual std::string stringify() const;

    /**
     * @brief Removes all elements from the object.
     */
    virtual void clear();

    // STL-like accessors
    /**
     * @brief Returns the number of elements in the object.
     * @return The number of key-value pairs.
     */
    size_type size() const noexcept;

    /**
     * @brief Checks if the object is empty.
     * @return true if the object contains no elements, false otherwise.
     */
    bool empty() const noexcept;

    /**
     * @brief Counts elements with a specific key.
     * @param key The key to search for.
     * @return 1 if the key exists, 0 otherwise.
     */
    size_type count(const std::string& key) const;

    /**
     * @brief Checks if a key exists in the object.
     * @param key The key to search for.
     * @return true if the key exists, false otherwise.
     */
    bool contains(const std::string& key) const;

    /**
     * @brief Returns an iterator to the beginning.
     * @return Iterator to the first element.
     */
    iterator begin() noexcept;

    /**
     * @brief Returns a const iterator to the beginning.
     * @return Const iterator to the first element.
     */
    const_iterator begin() const noexcept;

    /**
     * @brief Returns a const iterator to the beginning.
     * @return Const iterator to the first element.
     */
    const_iterator cbegin() const noexcept;

    /**
     * @brief Returns an iterator to the end.
     * @return Iterator to one past the last element.
     */
    iterator end() noexcept;

    /**
     * @brief Returns a const iterator to the end.
     * @return Const iterator to one past the last element.
     */
    const_iterator end() const noexcept;

    /**
     * @brief Returns a const iterator to the end.
     * @return Const iterator to one past the last element.
     */
    const_iterator cend() const noexcept;

    /**
     * @brief Finds an element by key.
     * @param key The key to search for.
     * @return Iterator to the element, or end() if not found.
     */
    iterator find(const std::string& key);

    /**
     * @brief Finds an element by key (const version).
     * @param key The key to search for.
     * @return Const iterator to the element, or end() if not found.
     */
    const_iterator find(const std::string& key) const;

    // Legacy compatibility
    /**
     * @brief Gets the internal data map (for backward compatibility).
     * @return A const reference to the internal unordered_map.
     * @deprecated Use iterators or STL-like methods instead.
     */
    const std::unordered_map<std::string, std::shared_ptr<json_object>>& get_data() const;

    /**
     * @brief Checks if a key exists (legacy name).
     * @param key The key to search for.
     * @return true if the key exists, false otherwise.
     * @deprecated Use contains() instead.
     */
    bool has_key(const std::string& key) const;

    // Operators
    /**
     * @brief Accesses or inserts an element.
     * @param key The key of the element.
     * @return A reference to the mapped value.
     * @note Creates a new empty json_object if the key doesn't exist.
     */
    std::shared_ptr<json_object>& operator[](const std::string& key);
};

}  // namespace cppress