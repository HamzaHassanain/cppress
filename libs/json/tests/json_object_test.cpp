#include <gtest/gtest.h>
#include "../json-parser.hpp"
#include <memory>
#include <string>

using namespace hh_json;

class JsonObjectTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Set up test fixtures if needed

        // testing::internal::CaptureStdout();

        // testing::internal::CaptureStderr();

        // // Uncomment below to see debug output during tests

        // std::string output = testing::internal::GetCapturedStdout();
        // if (!output.empty())
        // {
        //     std::cout << "Captured stdout during test setup: " << output << std::endl;
        // }
    }
};

TEST_F(JsonObjectTest, DefaultConstructor)
{
    auto obj = std::make_shared<JsonObject>();
    EXPECT_EQ(obj->get_data().size(), 0);
    EXPECT_EQ(obj->stringify(), "{}");
}

TEST_F(JsonObjectTest, InsertAndRetrieve)
{
    auto obj = std::make_shared<JsonObject>();
    auto str_val = std::make_shared<JsonString>("test_value");

    obj->insert("test_key", str_val);
    auto retrieved = obj->get("test_key");

    ASSERT_NE(retrieved, nullptr);
    auto retrieved_str = std::dynamic_pointer_cast<JsonString>(retrieved);
    ASSERT_NE(retrieved_str, nullptr);
    EXPECT_EQ(retrieved_str->value, "test_value");
}

TEST_F(JsonObjectTest, HasKey)
{
    auto obj = std::make_shared<JsonObject>();
    auto bool_val = std::make_shared<JsonBoolean>(true);

    EXPECT_FALSE(obj->has_key("test_key"));
    obj->insert("test_key", bool_val);
    EXPECT_TRUE(obj->has_key("test_key"));
}

TEST_F(JsonObjectTest, EraseKey)
{
    auto obj = std::make_shared<JsonObject>();
    auto num_val = std::make_shared<JsonNumber>(42.0);

    obj->insert("test_key", num_val);
    EXPECT_TRUE(obj->has_key("test_key"));

    obj->erase("test_key");
    EXPECT_FALSE(obj->has_key("test_key"));
    EXPECT_EQ(obj->get("test_key"), nullptr);
}

TEST_F(JsonObjectTest, MultipleInsertions)
{
    auto obj = std::make_shared<JsonObject>();

    obj->insert("string_key", std::make_shared<JsonString>("hello"));
    obj->insert("number_key", std::make_shared<JsonNumber>(123));
    obj->insert("boolean_key", std::make_shared<JsonBoolean>(true));

    EXPECT_EQ(obj->get_data().size(), 3);
    EXPECT_TRUE(obj->has_key("string_key"));
    EXPECT_TRUE(obj->has_key("number_key"));
    EXPECT_TRUE(obj->has_key("boolean_key"));
}

TEST_F(JsonObjectTest, OverwriteExistingKey)
{
    auto obj = std::make_shared<JsonObject>();

    obj->insert("key", std::make_shared<JsonString>("original"));
    auto retrieved1 = std::dynamic_pointer_cast<JsonString>(obj->get("key"));
    ASSERT_NE(retrieved1, nullptr);
    EXPECT_EQ(retrieved1->value, "original");

    // Overwrite with new value
    obj->insert("key", std::make_shared<JsonString>("updated"));
    auto retrieved2 = std::dynamic_pointer_cast<JsonString>(obj->get("key"));
    ASSERT_NE(retrieved2, nullptr);
    EXPECT_EQ(retrieved2->value, "updated");
}

TEST_F(JsonObjectTest, GetNonExistentKey)
{
    auto obj = std::make_shared<JsonObject>();
    auto result = obj->get("non_existent");
    EXPECT_EQ(result, nullptr);
}

TEST_F(JsonObjectTest, NestedObjects)
{
    auto parent = std::make_shared<JsonObject>();
    auto child = std::make_shared<JsonObject>();

    child->insert("child_key", std::make_shared<JsonString>("child_value"));
    parent->insert("nested", child);

    auto retrieved_child = std::dynamic_pointer_cast<JsonObject>(parent->get("nested"));
    ASSERT_NE(retrieved_child, nullptr);

    auto child_value = std::dynamic_pointer_cast<JsonString>(retrieved_child->get("child_key"));
    ASSERT_NE(child_value, nullptr);
    EXPECT_EQ(child_value->value, "child_value");
}

TEST_F(JsonObjectTest, ObjectWithArray)
{
    auto obj = std::make_shared<JsonObject>();
    auto arr = std::make_shared<JsonArray>();

    arr->insert(std::make_shared<JsonString>("item1"));
    arr->insert(std::make_shared<JsonString>("item2"));

    obj->insert("array_key", arr);

    auto retrieved_arr = std::dynamic_pointer_cast<JsonArray>(obj->get("array_key"));
    ASSERT_NE(retrieved_arr, nullptr);
    EXPECT_EQ(retrieved_arr->elements.size(), 2);
}

TEST_F(JsonObjectTest, StringifyEmptyObject)
{
    auto obj = std::make_shared<JsonObject>();
    EXPECT_EQ(obj->stringify(), "{}");
}

TEST_F(JsonObjectTest, StringifySimpleObject)
{
    auto obj = std::make_shared<JsonObject>();
    obj->insert("name", std::make_shared<JsonString>("John"));
    obj->insert("age", std::make_shared<JsonNumber>(30));

    std::string result = obj->stringify();

    // print the result to the console for debugging

    EXPECT_NE(result.find("\"name\": \"John\""), std::string::npos);
    EXPECT_NE(result.find("\"age\": 30"), std::string::npos);
    EXPECT_NE(result.find("{"), std::string::npos);
    EXPECT_NE(result.find("}"), std::string::npos);
}

TEST_F(JsonObjectTest, SetJsonDataThrowsException)
{
    auto obj = std::make_shared<JsonObject>();
    // JsonObject doesn't implement set_json_data, so it should throw
    EXPECT_THROW(obj->set_json_data("some data"), std::runtime_error);
}

TEST_F(JsonObjectTest, LargeObject)
{
    auto obj = std::make_shared<JsonObject>();

    // Insert many key-value pairs
    for (int i = 0; i < 100; ++i)
    {
        std::string key = "key_" + std::to_string(i);
        obj->insert(key, std::make_shared<JsonNumber>(i));
    }

    EXPECT_EQ(obj->get_data().size(), 100);

    // Check a few specific keys
    auto val_50 = std::dynamic_pointer_cast<JsonNumber>(obj->get("key_50"));
    ASSERT_NE(val_50, nullptr);
    EXPECT_DOUBLE_EQ(val_50->value, 50.0);
}

TEST_F(JsonObjectTest, KeysWithSpecialCharacters)
{
    auto obj = std::make_shared<JsonObject>();

    obj->insert("key with spaces", std::make_shared<JsonString>("value1"));
    obj->insert("key_with_underscores", std::make_shared<JsonString>("value2"));
    obj->insert("key-with-dashes", std::make_shared<JsonString>("value3"));
    obj->insert("key.with.dots", std::make_shared<JsonString>("value4"));
    obj->insert("key/with/slashes", std::make_shared<JsonString>("value5"));

    EXPECT_TRUE(obj->has_key("key with spaces"));
    EXPECT_TRUE(obj->has_key("key_with_underscores"));
    EXPECT_TRUE(obj->has_key("key-with-dashes"));
    EXPECT_TRUE(obj->has_key("key.with.dots"));
    EXPECT_TRUE(obj->has_key("key/with/slashes"));
}

TEST_F(JsonObjectTest, EmptyKeyHandling)
{
    auto obj = std::make_shared<JsonObject>();

    // Insert with empty key
    obj->insert("", std::make_shared<JsonString>("empty_key_value"));

    EXPECT_TRUE(obj->has_key(""));
    auto retrieved = std::dynamic_pointer_cast<JsonString>(obj->get(""));
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->value, "empty_key_value");
}

TEST_F(JsonObjectTest, DeepNesting)
{
    auto level1 = std::make_shared<JsonObject>();
    auto level2 = std::make_shared<JsonObject>();
    auto level3 = std::make_shared<JsonObject>();

    level3->insert("deep_value", std::make_shared<JsonString>("found"));
    level2->insert("level3", level3);
    level1->insert("level2", level2);

    // Navigate through the nesting
    auto l2 = std::dynamic_pointer_cast<JsonObject>(level1->get("level2"));
    ASSERT_NE(l2, nullptr);

    auto l3 = std::dynamic_pointer_cast<JsonObject>(l2->get("level3"));
    ASSERT_NE(l3, nullptr);

    auto deep_val = std::dynamic_pointer_cast<JsonString>(l3->get("deep_value"));
    ASSERT_NE(deep_val, nullptr);
    EXPECT_EQ(deep_val->value, "found");
}
