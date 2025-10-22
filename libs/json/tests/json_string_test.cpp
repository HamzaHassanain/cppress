#include <gtest/gtest.h>
#include "../json-parser.hpp"
#include <memory>
#include <string>

using namespace hh_json;

class JsonStringTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Set up test fixtures if needed
    }
};

TEST_F(JsonStringTest, ConstructorWithValue)
{
    auto json_str = std::make_shared<JsonString>("Hello World");
    EXPECT_EQ(json_str->value, "Hello World");
    EXPECT_EQ(json_str->stringify(), "\"Hello World\"");
}

TEST_F(JsonStringTest, DefaultConstructor)
{
    auto json_str = std::make_shared<JsonString>();
    EXPECT_EQ(json_str->value, "");
    EXPECT_EQ(json_str->stringify(), "\"\"");
}

TEST_F(JsonStringTest, SetJsonData)
{
    auto json_str = std::make_shared<JsonString>();
    EXPECT_TRUE(json_str->set_json_data("Test String"));
    EXPECT_EQ(json_str->value, "Test String");
    EXPECT_EQ(json_str->stringify(), "\"Test String\"");
}

TEST_F(JsonStringTest, SetJsonDataEmpty)
{
    auto json_str = std::make_shared<JsonString>();
    EXPECT_TRUE(json_str->set_json_data(""));
    EXPECT_EQ(json_str->value, "");
    EXPECT_EQ(json_str->stringify(), "\"\"");
}

TEST_F(JsonStringTest, SetJsonDataWithSpecialCharacters)
{
    auto json_str = std::make_shared<JsonString>();

    // Test with newlines, tabs, quotes
    EXPECT_TRUE(json_str->set_json_data("Line 1\nLine 2"));
    EXPECT_EQ(json_str->value, "Line 1\nLine 2");

    EXPECT_TRUE(json_str->set_json_data("Tab\tSeparated"));
    EXPECT_EQ(json_str->value, "Tab\tSeparated");

    EXPECT_TRUE(json_str->set_json_data("Quote: \"Hello\""));
    EXPECT_EQ(json_str->value, "Quote: \"Hello\"");
}

TEST_F(JsonStringTest, StringifyEscaping)
{
    auto json_str1 = std::make_shared<JsonString>("Hello\nWorld");
    std::string result1 = json_str1->stringify();
    EXPECT_NE(result1.find("\\n"), std::string::npos);

    auto json_str2 = std::make_shared<JsonString>("Quote: \"test\"");
    std::string result2 = json_str2->stringify();
    EXPECT_NE(result2.find("\\\""), std::string::npos);
}

TEST_F(JsonStringTest, GetMethodThrowsException)
{
    auto json_str = std::make_shared<JsonString>("test");
    EXPECT_THROW(json_str->get("any_key"), std::runtime_error);
}

TEST_F(JsonStringTest, LongStrings)
{
    std::string long_str(1000, 'A');
    auto json_str = std::make_shared<JsonString>(long_str);
    EXPECT_EQ(json_str->value, long_str);

    std::string stringified = json_str->stringify();
    EXPECT_EQ(stringified.length(), long_str.length() + 2); // +2 for quotes
}

TEST_F(JsonStringTest, UnicodeCharacters)
{
    auto json_str = std::make_shared<JsonString>("Hello 世界 🌍");
    EXPECT_EQ(json_str->value, "Hello 世界 🌍");
    EXPECT_EQ(json_str->stringify(), "\"Hello 世界 🌍\"");
}

TEST_F(JsonStringTest, StringifyConsistency)
{
    auto json_str = std::make_shared<JsonString>("Consistent");

    // Multiple calls should return the same result
    std::string result1 = json_str->stringify();
    std::string result2 = json_str->stringify();
    EXPECT_EQ(result1, result2);
}

// Integration tests
TEST_F(JsonStringTest, JsonStringInObject)
{
    auto obj = std::make_shared<JsonObject>();
    auto str1 = std::make_shared<JsonString>("value1");
    auto str2 = std::make_shared<JsonString>("value2");

    obj->insert("key1", str1);
    obj->insert("key2", str2);

    auto retrieved1 = std::dynamic_pointer_cast<JsonString>(obj->get("key1"));
    auto retrieved2 = std::dynamic_pointer_cast<JsonString>(obj->get("key2"));

    ASSERT_NE(retrieved1, nullptr);
    ASSERT_NE(retrieved2, nullptr);

    EXPECT_EQ(retrieved1->value, "value1");
    EXPECT_EQ(retrieved2->value, "value2");
}

TEST_F(JsonStringTest, JsonStringInArray)
{
    auto arr = std::make_shared<JsonArray>();
    arr->insert(std::make_shared<JsonString>("first"));
    arr->insert(std::make_shared<JsonString>("second"));
    arr->insert(std::make_shared<JsonString>("third"));

    EXPECT_EQ(arr->elements.size(), 3);

    auto elem1 = std::dynamic_pointer_cast<JsonString>(arr->elements[0]);
    auto elem2 = std::dynamic_pointer_cast<JsonString>(arr->elements[1]);
    auto elem3 = std::dynamic_pointer_cast<JsonString>(arr->elements[2]);

    ASSERT_NE(elem1, nullptr);
    ASSERT_NE(elem2, nullptr);
    ASSERT_NE(elem3, nullptr);

    EXPECT_EQ(elem1->value, "first");
    EXPECT_EQ(elem2->value, "second");
    EXPECT_EQ(elem3->value, "third");
}

TEST_F(JsonStringTest, ModifyAfterCreation)
{
    auto json_str = std::make_shared<JsonString>("initial");
    EXPECT_EQ(json_str->value, "initial");

    // Modify the value directly
    json_str->value = "modified";
    EXPECT_EQ(json_str->value, "modified");
    EXPECT_EQ(json_str->stringify(), "\"modified\"");
}
