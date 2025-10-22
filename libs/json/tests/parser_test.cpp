#include <gtest/gtest.h>
#include "../json-parser.hpp"
#include <memory>
#include <string>

using namespace hh_json;

class ParserTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Set up test fixtures if needed
    }
};

// Test basic JSON object parsing
TEST_F(ParserTest, ParseSimpleObject)
{
    const std::string json = R"({"name": "John", "age": 25})";

    try
    {
        auto parsed = parse(json);

        ASSERT_NE(parsed.find("name"), parsed.end());
        ASSERT_NE(parsed.find("age"), parsed.end());

        auto name_obj = std::dynamic_pointer_cast<JsonString>(parsed["name"]);
        auto age_obj = std::dynamic_pointer_cast<JsonNumber>(parsed["age"]);

        ASSERT_NE(name_obj, nullptr);
        ASSERT_NE(age_obj, nullptr);

        EXPECT_EQ(name_obj->value, "John");
        EXPECT_DOUBLE_EQ(age_obj->value, 25.0);
    }
    catch (const std::exception &e)
    {
        FAIL() << "Parsing failed: " << e.what();
    }
}

// Test JSON object with boolean values
TEST_F(ParserTest, ParseObjectWithBooleans)
{
    const std::string json = R"({"active": true, "deleted": false})";

    try
    {
        auto parsed = parse(json);

        auto active_obj = std::dynamic_pointer_cast<JsonBoolean>(parsed["active"]);
        auto deleted_obj = std::dynamic_pointer_cast<JsonBoolean>(parsed["deleted"]);

        ASSERT_NE(active_obj, nullptr);
        ASSERT_NE(deleted_obj, nullptr);

        EXPECT_TRUE(active_obj->value);
        EXPECT_FALSE(deleted_obj->value);
    }
    catch (const std::exception &e)
    {
        FAIL() << "Parsing failed: " << e.what();
    }
}

// Test JSON object with array
TEST_F(ParserTest, ParseObjectWithArray)
{
    const std::string json = R"({"numbers": [1, 2, 3], "strings": ["a", "b"]})";

    try
    {
        auto parsed = parse(json);

        auto numbers_obj = std::dynamic_pointer_cast<JsonArray>(parsed["numbers"]);
        auto strings_obj = std::dynamic_pointer_cast<JsonArray>(parsed["strings"]);

        ASSERT_NE(numbers_obj, nullptr);
        ASSERT_NE(strings_obj, nullptr);

        EXPECT_EQ(numbers_obj->elements.size(), 3);
        EXPECT_EQ(strings_obj->elements.size(), 2);

        // Check number array elements
        auto num1 = std::dynamic_pointer_cast<JsonNumber>(numbers_obj->elements[0]);
        auto num2 = std::dynamic_pointer_cast<JsonNumber>(numbers_obj->elements[1]);
        auto num3 = std::dynamic_pointer_cast<JsonNumber>(numbers_obj->elements[2]);

        ASSERT_NE(num1, nullptr);
        ASSERT_NE(num2, nullptr);
        ASSERT_NE(num3, nullptr);

        EXPECT_DOUBLE_EQ(num1->value, 1.0);
        EXPECT_DOUBLE_EQ(num2->value, 2.0);
        EXPECT_DOUBLE_EQ(num3->value, 3.0);

        // Check string array elements
        auto str1 = std::dynamic_pointer_cast<JsonString>(strings_obj->elements[0]);
        auto str2 = std::dynamic_pointer_cast<JsonString>(strings_obj->elements[1]);

        ASSERT_NE(str1, nullptr);
        ASSERT_NE(str2, nullptr);

        EXPECT_EQ(str1->value, "a");
        EXPECT_EQ(str2->value, "b");
    }
    catch (const std::exception &e)
    {
        FAIL() << "Parsing failed: " << e.what();
    }
}

// Test nested objects
TEST_F(ParserTest, ParseNestedObjects)
{
    const std::string json = R"({
        "user": {
            "name": "Alice",
            "profile": {
                "age": 30,
                "active": true
            }
        }
    })";

    try
    {
        auto parsed = parse(json);

        auto user_obj = std::dynamic_pointer_cast<JsonObject>(parsed["user"]);
        ASSERT_NE(user_obj, nullptr);

        auto name_obj = std::dynamic_pointer_cast<JsonString>(user_obj->get("name"));
        ASSERT_NE(name_obj, nullptr);
        EXPECT_EQ(name_obj->value, "Alice");

        auto profile_obj = std::dynamic_pointer_cast<JsonObject>(user_obj->get("profile"));
        ASSERT_NE(profile_obj, nullptr);

        auto age_obj = std::dynamic_pointer_cast<JsonNumber>(profile_obj->get("age"));
        auto active_obj = std::dynamic_pointer_cast<JsonBoolean>(profile_obj->get("active"));

        ASSERT_NE(age_obj, nullptr);
        ASSERT_NE(active_obj, nullptr);

        EXPECT_DOUBLE_EQ(age_obj->value, 30.0);
        EXPECT_TRUE(active_obj->value);
    }
    catch (const std::exception &e)
    {
        FAIL() << "Parsing failed: " << e.what();
    }
}

// Test empty object and array
TEST_F(ParserTest, ParseEmptyStructures)
{
    const std::string json = R"({"empty_obj": {}, "empty_arr": []})";

    try
    {
        auto parsed = parse(json);

        auto empty_obj = std::dynamic_pointer_cast<JsonObject>(parsed["empty_obj"]);
        auto empty_arr = std::dynamic_pointer_cast<JsonArray>(parsed["empty_arr"]);

        ASSERT_NE(empty_obj, nullptr);
        ASSERT_NE(empty_arr, nullptr);

        EXPECT_EQ(empty_obj->get_data().size(), 0);
        EXPECT_EQ(empty_arr->elements.size(), 0);
    }
    catch (const std::exception &e)
    {
        FAIL() << "Parsing failed: " << e.what();
    }
}

// Test parsing numbers (integers, floats, negative)
TEST_F(ParserTest, ParseNumbers)
{
    const std::string json = R"({
        "integer": 42,
        "float": 3.14159,
        "negative": -123,
        "negative_float": -45.67,
        "zero": 0
    })";

    try
    {
        auto parsed = parse(json);

        auto integer_obj = std::dynamic_pointer_cast<JsonNumber>(parsed["integer"]);
        auto float_obj = std::dynamic_pointer_cast<JsonNumber>(parsed["float"]);
        auto negative_obj = std::dynamic_pointer_cast<JsonNumber>(parsed["negative"]);
        auto negative_float_obj = std::dynamic_pointer_cast<JsonNumber>(parsed["negative_float"]);
        auto zero_obj = std::dynamic_pointer_cast<JsonNumber>(parsed["zero"]);

        ASSERT_NE(integer_obj, nullptr);
        ASSERT_NE(float_obj, nullptr);
        ASSERT_NE(negative_obj, nullptr);
        ASSERT_NE(negative_float_obj, nullptr);
        ASSERT_NE(zero_obj, nullptr);

        EXPECT_DOUBLE_EQ(integer_obj->value, 42.0);
        EXPECT_DOUBLE_EQ(float_obj->value, 3.14159);
        EXPECT_DOUBLE_EQ(negative_obj->value, -123.0);
        EXPECT_DOUBLE_EQ(negative_float_obj->value, -45.67);
        EXPECT_DOUBLE_EQ(zero_obj->value, 0.0);
    }
    catch (const std::exception &e)
    {
        FAIL() << "Parsing failed: " << e.what();
    }
}

// Test string escapes
TEST_F(ParserTest, ParseStringEscapes)
{
    const std::string json = R"({
        "newline": "Hello\nWorld",
        "tab": "Hello\tWorld",
        "quote": "Say \"Hello\"",
        "backslash": "Path\\to\\file"
    })";

    try
    {
        auto parsed = parse(json);

        auto newline_obj = std::dynamic_pointer_cast<JsonString>(parsed["newline"]);
        auto tab_obj = std::dynamic_pointer_cast<JsonString>(parsed["tab"]);
        auto quote_obj = std::dynamic_pointer_cast<JsonString>(parsed["quote"]);
        auto backslash_obj = std::dynamic_pointer_cast<JsonString>(parsed["backslash"]);

        ASSERT_NE(newline_obj, nullptr);
        ASSERT_NE(tab_obj, nullptr);
        ASSERT_NE(quote_obj, nullptr);
        ASSERT_NE(backslash_obj, nullptr);

        EXPECT_EQ(newline_obj->value, "Hello\nWorld");
        EXPECT_EQ(tab_obj->value, "Hello\tWorld");
        EXPECT_EQ(quote_obj->value, "Say \"Hello\"");
        EXPECT_EQ(backslash_obj->value, "Path\\to\\file");
    }
    catch (const std::exception &e)
    {
        FAIL() << "Parsing failed: " << e.what();
    }
}

// Test JsonValue function
TEST_F(ParserTest, JsonValueFunction)
{
    // Test string value
    auto str_val = JsonValue("\"test string\"");
    auto str_cast = std::dynamic_pointer_cast<JsonString>(str_val);
    ASSERT_NE(str_cast, nullptr);
    EXPECT_EQ(str_cast->value, "test string");

    // Test number value
    auto num_val = JsonValue("42.5");
    auto num_cast = std::dynamic_pointer_cast<JsonNumber>(num_val);
    ASSERT_NE(num_cast, nullptr);
    EXPECT_DOUBLE_EQ(num_cast->value, 42.5);

    // Test boolean value
    auto bool_val = JsonValue("true");
    auto bool_cast = std::dynamic_pointer_cast<JsonBoolean>(bool_val);
    ASSERT_NE(bool_cast, nullptr);
    EXPECT_TRUE(bool_cast->value);

    // Test array value
    auto arr_val = JsonValue("[1, 2, 3]");
    auto arr_cast = std::dynamic_pointer_cast<JsonArray>(arr_val);
    ASSERT_NE(arr_cast, nullptr);
    EXPECT_EQ(arr_cast->elements.size(), 3);
}

// Test error handling
TEST_F(ParserTest, ParseInvalidJson)
{
    // Test various invalid JSON strings
    std::vector<std::string> invalid_jsons = {
        R"({"key": })",                     // Missing value
        R"({"key" "value"})",               // Missing colon
        R"({key: "value"})",                // Unquoted key
        R"({"key": "value",})",             // Trailing comma
        R"({"key": "unterminated string})", // Unterminated string
        R"([1, 2, 3,])",                    // Trailing comma in array
        R"({"key": [1, 2, 3})",             // Mismatched brackets
    };

    for (const auto &invalid_json : invalid_jsons)
    {
        EXPECT_THROW(parse(invalid_json), std::exception)
            << "Should throw exception for: " << invalid_json;
    }
}

// Test complex real-world-like JSON
TEST_F(ParserTest, ParseComplexJson)
{
    const std::string json = R"({
        "user": {
            "id": 12345,
            "username": "john_doe",
            "profile": {
                "firstName": "John",
                "lastName": "Doe",
                "age": 30,
                "isActive": true,
                "preferences": {
                    "theme": "dark",
                    "notifications": false
                }
            },
            "roles": ["user", "admin"],
            "scores": [85.5, 92.0, 78.3]
        },
        "metadata": {
            "version": "1.0",
            "timestamp": 1609459200,
            "features": ["auth", "api", "dashboard"]
        }
    })";

    try
    {
        auto parsed = parse(json);

        // Verify user object
        auto user_obj = std::dynamic_pointer_cast<JsonObject>(parsed["user"]);
        ASSERT_NE(user_obj, nullptr);

        // Check user ID
        auto id_obj = std::dynamic_pointer_cast<JsonNumber>(user_obj->get("id"));
        ASSERT_NE(id_obj, nullptr);
        EXPECT_DOUBLE_EQ(id_obj->value, 12345.0);

        // Check roles array
        auto roles_obj = std::dynamic_pointer_cast<JsonArray>(user_obj->get("roles"));
        ASSERT_NE(roles_obj, nullptr);
        EXPECT_EQ(roles_obj->elements.size(), 2);

        // Check nested preferences
        auto profile_obj = std::dynamic_pointer_cast<JsonObject>(user_obj->get("profile"));
        ASSERT_NE(profile_obj, nullptr);

        auto prefs_obj = std::dynamic_pointer_cast<JsonObject>(profile_obj->get("preferences"));
        ASSERT_NE(prefs_obj, nullptr);

        auto theme_obj = std::dynamic_pointer_cast<JsonString>(prefs_obj->get("theme"));
        ASSERT_NE(theme_obj, nullptr);
        EXPECT_EQ(theme_obj->value, "dark");
    }
    catch (const std::exception &e)
    {
        FAIL() << "Parsing failed: " << e.what();
    }
}
