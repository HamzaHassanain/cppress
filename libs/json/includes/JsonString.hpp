#pragma once

#include <string>
#include "JsonObject.hpp"

namespace hh_json
{
    class JsonString : public JsonObject
    {

    public:
        std::string value;
        JsonString() = default;

        JsonString(const std::string &value) : value(value) {}
        ~JsonString() = default;

        virtual std::shared_ptr<JsonObject> get([[maybe_unused]] const std::string &key) const
        {
            throw std::runtime_error("JsonString does not contain objects");
        }
        bool set_json_data(const std::string &jsonString) override
        {
            value = jsonString;
            return true;
        }
        std::string stringify() const override
        {
            // Handle escape characters properly
            std::string escaped = value;
            size_t pos = 0;

            // Escape backslashes first (must be done before other escapes)
            while ((pos = escaped.find("\\", pos)) != std::string::npos)
            {
                escaped.replace(pos, 1, "\\\\");
                pos += 2; // Skip the newly inserted characters
            }

            // Escape double quotes
            pos = 0;
            while ((pos = escaped.find("\"", pos)) != std::string::npos)
            {
                escaped.replace(pos, 1, "\\\"");
                pos += 2; // Skip the newly inserted characters
            }

            // Escape other common characters
            pos = 0;
            while ((pos = escaped.find("\n", pos)) != std::string::npos)
            {
                escaped.replace(pos, 1, "\\n");
                pos += 2;
            }

            pos = 0;
            while ((pos = escaped.find("\r", pos)) != std::string::npos)
            {
                escaped.replace(pos, 1, "\\r");
                pos += 2;
            }

            pos = 0;
            while ((pos = escaped.find("\t", pos)) != std::string::npos)
            {
                escaped.replace(pos, 1, "\\t");
                pos += 2;
            }

            pos = 0;
            while ((pos = escaped.find("\b", pos)) != std::string::npos)
            {
                escaped.replace(pos, 1, "\\b");
                pos += 2;
            }

            pos = 0;
            while ((pos = escaped.find("\f", pos)) != std::string::npos)
            {
                escaped.replace(pos, 1, "\\f");
                pos += 2;
            }

            return "\"" + escaped + "\"";
        }

    private:
    };
}