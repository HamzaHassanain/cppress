
#pragma once

#include <string>
#include <unordered_map>
#include <memory>

namespace hh_json
{
    class JsonObject;

    std::shared_ptr<JsonObject> JsonValue(const std::string &valueString);

    // Main parsing function to parse a JSON string into a map of JSON objects
    std::unordered_map<std::string, std::shared_ptr<JsonObject>>
    parse(const std::string &jsonString);
}
