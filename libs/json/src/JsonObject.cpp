#include <iostream>

#include "../includes/JsonObject.hpp"
#include "../includes/parser.hpp"
namespace hh_json
{

    JsonObject::JsonObject() = default;
    JsonObject::~JsonObject() = default;
    JsonObject::JsonObject(const std::unordered_map<std::string, std::shared_ptr<JsonObject>> &initial_data) : data(initial_data) {}

    bool JsonObject::set_json_data(const std::string &jsonString)
    {
        try
        {

            data.clear();
            data = hh_json::parse(jsonString);
            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error parsing JSON data: " << e.what() << std::endl;
            // Re-throw as runtime_error for tests that expect exceptions
            throw std::runtime_error("Failed to parse JSON data: " + std::string(e.what()));
        }
    }

    std::string JsonObject::stringify() const
    {
        std::string result = "{";

        // Add key-value pairs to the JSON object
        for (const auto &pair : data)
        {
            if (!pair.second)
            {
                result += "\"" + pair.first + "\": " + "{}" + ",";
            }
            else
                result += "\"" + pair.first + "\": " + pair.second->stringify() + ",";
        }
        if (!data.empty())
        {
            result.pop_back(); // Remove trailing comma
        }
        result += "}";
        return result;
    }

    void JsonObject::insert(const std::string &key, std::shared_ptr<JsonObject> value)
    {
        data[key] = value; // This will overwrite existing keys
    }

    void JsonObject::erase(const std::string &key)
    {
        data.erase(key);
    }

    std::shared_ptr<JsonObject> JsonObject::get(const std::string &key) const
    {
        auto it = data.find(key);
        if (it != data.end())
        {
            return it->second;
        }
        return nullptr; // Return a nullptr if key not found
    }

    void JsonObject::clear()
    {
        data.clear();
    }

    const std::unordered_map<std::string, std::shared_ptr<JsonObject>> &JsonObject::get_data() const
    {
        return data;
    }

    std::shared_ptr<JsonObject> &JsonObject::operator[](const std::string &key)
    {
        if (data.find(key) == data.end())
        {
            data[key] = std::make_shared<JsonObject>();
        }
        return data[key];
    }

    bool JsonObject::has_key(const std::string &key) const
    {
        return data.find(key) != data.end();
    }

}
