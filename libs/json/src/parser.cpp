#include <algorithm>
#include <vector>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

#include "../includes/parser.hpp"
#include "../includes/JsonObject.hpp"
#include "../includes/JsonArray.hpp"
#include "../includes/JsonString.hpp"
#include "../includes/JsonNumber.hpp"
#include "../includes/JsonBoolean.hpp"

namespace hh_json
{
    void trim(std::string &str)
    {
        str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch)
                                            { return !std::isspace(ch); }));
        str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch)
                               { return !std::isspace(ch); })
                      .base(),
                  str.end());
    }

    // favor space over time, as we parse in O of N time, and O of N space (take copy of the string)
    void remove_spaces_not_in_string_literals(std::string &str)
    {
        bool in_string = false;
        std::string result;
        result.reserve(str.length()); // Pre-allocate memory for efficiency

        for (size_t i = 0; i < str.length(); ++i)
        {
            if (str[i] == '\"' && (i == 0 || str[i - 1] != '\\'))
            {
                in_string = !in_string;
                result.push_back(str[i]);
            }
            else if (!in_string && std::isspace(str[i]))
            {
                continue;
            }
            else
            {
                result.push_back(str[i]);
            }
        }
        str = std::move(result); // Use move for efficiency
    }

    // favor space over time, as we parse in O of N time, and O of N space (take copy of the string)
    void erase_comments(std::string &str)
    {
        std::string result;
        result.reserve(str.length()); // Pre-allocate for efficiency
        bool in_string = false;

        for (size_t i = 0; i < str.length(); ++i)
        {
            if (str[i] == '\"' && (i == 0 || str[i - 1] != '\\'))
            {
                in_string = !in_string;
                result.push_back(str[i]);
            }
            else if (!in_string && str[i] == '/' && i + 1 < str.length() && str[i + 1] == '/')
            {
                // Skip the comment to end of line
                while (i < str.length() && str[i] != '\n')
                {
                    ++i;
                }
                // Add a space in place of the comment
                result.push_back(' ');
            }
            else
            {
                result.push_back(str[i]);
            }
        }
        str = std::move(result); // Use move for efficiency
    }

    // Forward declarations of parsing functions
    std::shared_ptr<JsonObject> parse_value(const std::string &str, size_t &pos);
    std::shared_ptr<JsonObject> parse_object(const std::string &str, size_t &pos);
    std::shared_ptr<JsonArray> parse_array(const std::string &str, size_t &pos);
    std::shared_ptr<JsonString> parse_string(const std::string &str, size_t &pos);
    std::shared_ptr<JsonNumber> parse_number(const std::string &str, size_t &pos);
    std::shared_ptr<JsonBoolean> parse_boolean(const std::string &str, size_t &pos);
    std::shared_ptr<JsonObject> parse_null(const std::string &str, size_t &pos);

    // Skip whitespace
    void skip_whitespace(const std::string &str, size_t &pos)
    {
        while (pos < str.length() && std::isspace(str[pos]))
        {
            ++pos;
        }
    }

    // Parse a JSON string value
    std::shared_ptr<JsonString> parse_string(const std::string &str, size_t &pos)
    {
        if (str[pos] != '\"')
        {
            throw std::runtime_error("Expected '\"' at position " + std::to_string(pos));
        }

        std::string value;
        ++pos; // Skip opening quote

        while (pos < str.length())
        {
            char c = str[pos++];
            if (c == '\"')
            {
                // End of string
                return std::make_shared<JsonString>(value);
            }
            else if (c == '\\' && pos < str.length())
            {
                // Handle escape sequences
                char next = str[pos++];
                switch (next)
                {
                case '\"':
                    value += '\"';
                    break;
                case '\\':
                    value += '\\';
                    break;
                case '/':
                    value += '/';
                    break;
                case 'b':
                    value += '\b';
                    break;
                case 'f':
                    value += '\f';
                    break;
                case 'n':
                    value += '\n';
                    break;
                case 'r':
                    value += '\r';
                    break;
                case 't':
                    value += '\t';
                    break;
                case 'u':
                    // Handle Unicode escape - simplified version
                    if (pos + 4 <= str.length())
                    {
                        // Just add the Unicode escape sequence as is for now
                        value += "\\u";
                        for (int i = 0; i < 4; ++i)
                        {
                            value += str[pos++];
                        }
                    }
                    break;
                default:
                    value += next;
                    break;
                }
            }
            else
            {
                value += c;
            }
        }

        throw std::runtime_error("Unterminated string at position " + std::to_string(pos));
    }

    // Parse a JSON number
    std::shared_ptr<JsonNumber> parse_number(const std::string &str, size_t &pos)
    {
        size_t start = pos;
        [[maybe_unused]] bool has_decimal = false;
        [[maybe_unused]] bool has_exponent = false;

        // Handle negative sign
        if (pos < str.length() && str[pos] == '-')
        {
            ++pos;
        }

        // Parse digits before decimal point
        while (pos < str.length() && std::isdigit(str[pos]))
        {
            ++pos;
        }

        // Parse decimal point and following digits
        if (pos < str.length() && str[pos] == '.')
        {
            has_decimal = true;
            ++pos;
            while (pos < str.length() && std::isdigit(str[pos]))
            {
                ++pos;
            }
        }

        // Parse exponent
        if (pos < str.length() && (str[pos] == 'e' || str[pos] == 'E'))
        {
            has_exponent = true;
            ++pos;

            if (pos < str.length() && (str[pos] == '+' || str[pos] == '-'))
            {
                ++pos;
            }

            while (pos < str.length() && std::isdigit(str[pos]))
            {
                ++pos;
            }
        }

        // Extract the number string and convert to double
        std::string number_str = str.substr(start, pos - start);
        auto result = std::make_shared<JsonNumber>();
        if (!result->set_json_data(number_str))
        {
            throw std::runtime_error("Invalid number format at position " + std::to_string(start));
        }

        return result;
    }

    // Parse a JSON boolean value
    std::shared_ptr<JsonBoolean> parse_boolean(const std::string &str, size_t &pos)
    {
        if (pos + 4 <= str.length() && str.substr(pos, 4) == "true")
        {
            pos += 4;
            return std::make_shared<JsonBoolean>(true);
        }
        else if (pos + 5 <= str.length() && str.substr(pos, 5) == "false")
        {
            pos += 5;
            return std::make_shared<JsonBoolean>(false);
        }

        throw std::runtime_error("Expected 'true' or 'false' at position " + std::to_string(pos));
    }

    // Parse a JSON null value
    std::shared_ptr<JsonObject> parse_null(const std::string &str, size_t &pos)
    {
        if (pos + 4 <= str.length() && str.substr(pos, 4) == "null")
        {
            pos += 4;
            return nullptr; // Return nullptr for null values
        }

        throw std::runtime_error("Expected 'null' at position " + std::to_string(pos));
    }

    // Parse a JSON array
    std::shared_ptr<JsonArray> parse_array(const std::string &str, size_t &pos)
    {
        if (str[pos] != '[')
        {
            throw std::runtime_error("Expected '[' at position " + std::to_string(pos));
        }

        ++pos; // Skip '['
        skip_whitespace(str, pos);

        auto array = std::make_shared<JsonArray>();

        // Check for empty array
        if (pos < str.length() && str[pos] == ']')
        {
            ++pos; // Skip ']'
            return array;
        }

        while (pos < str.length())
        {
            // Parse array element
            auto element = parse_value(str, pos);
            array->insert(element);

            skip_whitespace(str, pos);

            // Check for end of array or next element
            if (pos < str.length() && str[pos] == ']')
            {
                ++pos; // Skip ']'
                return array;
            }

            if (pos < str.length() && str[pos] == ',')
            {
                ++pos; // Skip ','
                skip_whitespace(str, pos);
            }
            else
            {
                throw std::runtime_error("Expected ',' or ']' at position " + std::to_string(pos));
            }
        }

        throw std::runtime_error("Unterminated array at position " + std::to_string(pos));
    }

    // Parse a JSON object
    std::shared_ptr<JsonObject> parse_object(const std::string &str, size_t &pos)
    {
        if (str[pos] != '{')
        {
            throw std::runtime_error("Expected '{' at position " + std::to_string(pos));
        }

        ++pos; // Skip '{'
        skip_whitespace(str, pos);

        std::unordered_map<std::string, std::shared_ptr<JsonObject>> properties;

        // Check for empty object
        if (pos < str.length() && str[pos] == '}')
        {
            ++pos; // Skip '}'
            auto result = std::make_shared<JsonObject>();
            for (const auto &[key, value] : properties)
            {
                result->insert(key, value);
            }
            return result;
        }

        while (pos < str.length())
        {
            // Parse key (must be a string)
            if (str[pos] != '\"')
            {
                throw std::runtime_error("Expected string key at position " + std::to_string(pos));
            }

            auto key_obj = parse_string(str, pos);
            std::string key = key_obj->stringify();
            // Remove the quotes from the key
            key = key.substr(1, key.length() - 2);

            skip_whitespace(str, pos);

            // Parse colon
            if (pos >= str.length() || str[pos] != ':')
            {
                throw std::runtime_error("Expected ':' at position " + std::to_string(pos));
            }

            ++pos; // Skip ':'
            skip_whitespace(str, pos);

            // Parse value
            auto value = parse_value(str, pos);
            properties[key] = value;

            skip_whitespace(str, pos);

            // Check for end of object or next property
            if (pos < str.length() && str[pos] == '}')
            {
                ++pos; // Skip '}'
                auto result = std::make_shared<JsonObject>();
                for (const auto &[key, value] : properties)
                {
                    result->insert(key, value);
                }
                return result;
            }

            if (pos < str.length() && str[pos] == ',')
            {
                ++pos; // Skip ','
                skip_whitespace(str, pos);
            }
            else
            {
                throw std::runtime_error("Expected ',' or '}' at position " + std::to_string(pos));
            }
        }

        throw std::runtime_error("Unterminated object at position " + std::to_string(pos));
    }

    // Parse a JSON value (can be object, array, string, number, boolean, or null)
    std::shared_ptr<JsonObject> parse_value(const std::string &str, size_t &pos)
    {
        skip_whitespace(str, pos);

        if (pos >= str.length())
        {
            throw std::runtime_error("Unexpected end of input");
        }

        char c = str[pos];

        if (c == '{')
        {
            return parse_object(str, pos);
        }
        else if (c == '[')
        {
            return parse_array(str, pos);
        }
        else if (c == '\"')
        {
            return parse_string(str, pos);
        }
        else if (c == '-' || std::isdigit(c))
        {
            return parse_number(str, pos);
        }
        else if (c == 't' || c == 'f')
        {
            return parse_boolean(str, pos);
        }
        else if (c == 'n')
        {
            return parse_null(str, pos);
        }

        throw std::runtime_error("Unexpected character at position " + std::to_string(pos) + ": " + c);
    }

    std::unordered_map<std::string, std::shared_ptr<JsonObject>>
    parse(const std::string &jsonString)
    {
        auto JsonString_copy = jsonString;

        trim(JsonString_copy);
        remove_spaces_not_in_string_literals(JsonString_copy);
        erase_comments(JsonString_copy);

        size_t pos = 0;
        skip_whitespace(JsonString_copy, pos);

        if (pos >= JsonString_copy.length() || JsonString_copy[pos] != '{')
        {
            throw std::runtime_error("JSON must start with an object");
        }

        auto root_obj = parse_object(JsonString_copy, pos);
        return root_obj->get_data();
    }

    std::shared_ptr<JsonObject> JsonValue(const std::string &valueString)
    {
        try
        {

            size_t pos = 0;
            if (valueString.empty())
            {
                return std::make_shared<JsonObject>();
            }

            return parse_value(valueString, pos);
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
            return nullptr;
        }
    }

}
