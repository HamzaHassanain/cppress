#pragma once

#include "JsonObject.hpp"

namespace hh_json
{
    class JsonNumber : public JsonObject
    {

    public:
        double value;
        JsonNumber() = default;

        JsonNumber(double value) : value(value) {}
        ~JsonNumber() = default;

        virtual std::shared_ptr<JsonObject> get([[maybe_unused]] const std::string &key) const
        {
            throw std::runtime_error("JsonNumber does not contain objects");
        }

        bool set_json_data(const std::string &jsonString) override
        {
            if (jsonString.empty())
            {
                return false;
            }

            try
            {
                size_t pos = 0;
                double parsed_value = std::stod(jsonString, &pos);

                // Check if the entire string was consumed (no trailing characters)
                if (pos != jsonString.length())
                {
                    return false;
                }

                value = parsed_value;
                return true;
            }
            catch (...)
            {
                return false;
            }
        }
        std::string stringify() const override
        {
            if ((long long)value == value)
                return std::to_string((long long)value);
            return std::to_string(value);
        }

    private:
    };
}
