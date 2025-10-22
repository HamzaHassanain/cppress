#pragma once

#include "JsonObject.hpp"

#include <stdexcept>
#include <algorithm>

namespace hh_json
{
    class JsonBoolean : public JsonObject
    {

    public:
        bool value;
        JsonBoolean() = default;

        JsonBoolean(bool value) : value(value) {}
        ~JsonBoolean() = default;
        virtual std::shared_ptr<JsonObject> get([[maybe_unused]] const std::string &key) const
        {
            throw std::runtime_error("JsonBoolean does not contain objects");
        }
        bool set_json_data(const std::string &temp) override
        {
            auto jsonString = temp;
            std::transform(jsonString.begin(), jsonString.end(), jsonString.begin(), ::tolower);
            if (jsonString == "true")
            {
                value = true;
                return true;
            }
            else if (jsonString == "false")
            {
                value = false;
                return true;
            }
            return false;
        }
        std::string stringify() const override
        {
            return value ? "true" : "false";
        }

    private:
    };
}
