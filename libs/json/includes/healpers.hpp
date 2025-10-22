#pragma once

#include <string>
#include <memory>
#include <vector>

#include "JsonObject.hpp"
#include "JsonString.hpp"
#include "JsonNumber.hpp"
#include "JsonBoolean.hpp"
#include "JsonArray.hpp"

namespace hh_json::maker
{
    inline std::shared_ptr<hh_json::JsonObject> make_string(const std::string &value)
    {
        return std::make_shared<hh_json::JsonString>(value);
    }

    inline std::shared_ptr<hh_json::JsonObject> make_number(double value)
    {
        return std::make_shared<hh_json::JsonNumber>(value);
    }

    inline std::shared_ptr<hh_json::JsonObject> make_boolean(bool value)
    {
        return std::make_shared<hh_json::JsonBoolean>(value);
    }

}

namespace hh_json::getter
{
    inline bool get_boolean(const std::shared_ptr<hh_json::JsonObject> &obj)
    {
        if (auto boolean = std::dynamic_pointer_cast<hh_json::JsonBoolean>(obj))
        {
            return boolean->value;
        }
        throw std::runtime_error("Not a boolean");
    }

    inline double get_number(const std::shared_ptr<hh_json::JsonObject> &obj)
    {
        if (auto number = std::dynamic_pointer_cast<hh_json::JsonNumber>(obj))
        {
            return number->value;
        }
        throw std::runtime_error("Not a number");
    }

    inline std::string get_string(const std::shared_ptr<hh_json::JsonObject> &obj)
    {
        if (auto str = std::dynamic_pointer_cast<hh_json::JsonString>(obj))
        {
            return str->value;
        }
        throw std::runtime_error("Not a string");
    }

    inline std::vector<std::shared_ptr<hh_json::JsonObject>> get_array(const std::shared_ptr<hh_json::JsonObject> &obj)
    {
        if (auto array = std::dynamic_pointer_cast<hh_json::JsonArray>(obj))
        {
            return array->elements;
        }
        throw std::runtime_error("Not an array");
    }
}