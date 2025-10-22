#include "../includes/json_object.hpp"

#include <iostream>

#include "../includes/parser.hpp"
namespace cppress::json {

json_object::json_object() = default;
json_object::~json_object() = default;
json_object::json_object(
    const std::unordered_map<std::string, std::shared_ptr<json_object>>& initial_data)
    : data(initial_data) {}

bool json_object::set_json_data(const std::string& jsonString) {
    try {
        data.clear();
        data = parse(jsonString);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON data: " << e.what() << std::endl;
        // Re-throw as runtime_error for tests that expect exceptions
        throw std::runtime_error("Failed to parse JSON data: " + std::string(e.what()));
    }
}

std::string json_object::stringify() const {
    std::string result = "{";

    // Add key-value pairs to the JSON object
    for (const auto& pair : data) {
        if (!pair.second) {
            result += "\"" + pair.first + "\":null,";
        } else {
            result += "\"" + pair.first + "\":" + pair.second->stringify() + ",";
        }
    }
    if (!data.empty()) {
        result.pop_back();  // Remove trailing comma
    }
    result += "}";
    return result;
}

void json_object::insert(const std::string& key, std::shared_ptr<json_object> value) {
    data[key] = value;  // This will overwrite existing keys
}

json_object::size_type json_object::erase(const std::string& key) {
    return data.erase(key);
}

std::shared_ptr<json_object> json_object::get(const std::string& key) const {
    auto it = data.find(key);
    if (it != data.end()) {
        return it->second;
    }
    return nullptr;  // Return a nullptr if key not found
}

std::shared_ptr<json_object> json_object::at(const std::string& key) const {
    auto it = data.find(key);
    if (it != data.end()) {
        return it->second;
    }
    throw std::out_of_range("Key not found: " + key);
}

void json_object::clear() {
    data.clear();
}

// STL-like accessors
json_object::size_type json_object::size() const noexcept {
    return data.size();
}

bool json_object::empty() const noexcept {
    return data.empty();
}

json_object::size_type json_object::count(const std::string& key) const {
    return data.count(key);
}

bool json_object::contains(const std::string& key) const {
    return data.find(key) != data.end();
}

json_object::iterator json_object::begin() noexcept {
    return data.begin();
}

json_object::const_iterator json_object::begin() const noexcept {
    return data.begin();
}

json_object::const_iterator json_object::cbegin() const noexcept {
    return data.cbegin();
}

json_object::iterator json_object::end() noexcept {
    return data.end();
}

json_object::const_iterator json_object::end() const noexcept {
    return data.end();
}

json_object::const_iterator json_object::cend() const noexcept {
    return data.cend();
}

json_object::iterator json_object::find(const std::string& key) {
    return data.find(key);
}

json_object::const_iterator json_object::find(const std::string& key) const {
    return data.find(key);
}

// Legacy compatibility
const std::unordered_map<std::string, std::shared_ptr<json_object>>& json_object::get_data() const {
    return data;
}

std::shared_ptr<json_object>& json_object::operator[](const std::string& key) {
    if (data.find(key) == data.end()) {
        data[key] = std::make_shared<json_object>();
    }
    return data[key];
}

bool json_object::has_key(const std::string& key) const {
    return data.find(key) != data.end();
}

}  // namespace cppress::json
