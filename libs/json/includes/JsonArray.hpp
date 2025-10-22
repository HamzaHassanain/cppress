#pragma once

#include <vector>
#include <stdexcept>
#include <memory>
#include "JsonObject.hpp"
#include "parser.hpp"
namespace hh_json
{
    class JsonArray : public JsonObject
    {

    public:
        std::vector<std::shared_ptr<JsonObject>> elements;
        JsonArray() = default;
        JsonArray(const std::vector<std::shared_ptr<JsonObject>> &elements) : elements(elements) {}
        ~JsonArray() = default;

        virtual std::shared_ptr<JsonObject> get([[maybe_unused]] const std::string &key) const
        {
            try
            {
                int idx = std::stoi(key);

                if (idx < 0 || idx >= static_cast<int>(elements.size()))
                {
                    throw std::out_of_range("Invalid array index");
                }
                return elements[idx];
            }
            catch (const std::invalid_argument &)
            {
                throw std::runtime_error("JsonArray does not support key-based access with non-numeric keys");
            }
            catch (const std::out_of_range &)
            {
                throw std::runtime_error("JsonArray index out of range");
            }
        }

        bool set_json_data([[maybe_unused]] const std::string &jsonString) override
        {
            auto obj = hh_json::JsonValue(jsonString);
            if (!obj)
            {
                return false;
            }
            if (auto arr = std::dynamic_pointer_cast<JsonArray>(obj))
            {
                elements = arr->elements;
            }
            else
            {
                return false;
            }
            return true;
        }
        void insert(std::shared_ptr<JsonObject> value)
        {
            elements.push_back(value);
        }
        std::string stringify() const override
        {
            std::string result = "[";
            for (const auto &element : elements)
            {
                result += element->stringify() + ",";
            }
            if (result.back() == ',')
            {
                result.pop_back(); // Remove trailing comma
            }
            result += "]";
            return result;
        }

    private:
    };
}