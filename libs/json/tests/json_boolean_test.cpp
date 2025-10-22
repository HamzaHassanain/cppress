#include <gtest/gtest.h>
#include "../json-parser.hpp"
#include <memory>
#include <string>

using namespace hh_json;

// Test JsonBoolean functionality - Primary focus as requested
class JsonBooleanTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Set up test fixtures if needed
    }
};

TEST_F(JsonBooleanTest, ConstructorWithTrueValue)
{
    auto json_bool = std::make_shared<JsonBoolean>(true);
    EXPECT_EQ(json_bool->value, true);
    EXPECT_EQ(json_bool->stringify(), "true");
}

TEST_F(JsonBooleanTest, ConstructorWithFalseValue)
{
    auto json_bool = std::make_shared<JsonBoolean>(false);
    EXPECT_EQ(json_bool->value, false);
    EXPECT_EQ(json_bool->stringify(), "false");
}

TEST_F(JsonBooleanTest, DefaultConstructor)
{
    auto json_bool = std::make_shared<JsonBoolean>();
    // Default value should be false (default bool initialization)
    EXPECT_EQ(json_bool->value, false);
    EXPECT_EQ(json_bool->stringify(), "false");
}

TEST_F(JsonBooleanTest, SetJsonDataWithTrue)
{
    auto json_bool = std::make_shared<JsonBoolean>();
    EXPECT_TRUE(json_bool->set_json_data("true"));
    EXPECT_EQ(json_bool->value, true);
    EXPECT_EQ(json_bool->stringify(), "true");
}

TEST_F(JsonBooleanTest, SetJsonDataWithFalse)
{
    auto json_bool = std::make_shared<JsonBoolean>();
    EXPECT_TRUE(json_bool->set_json_data("false"));
    EXPECT_EQ(json_bool->value, false);
    EXPECT_EQ(json_bool->stringify(), "false");
}

TEST_F(JsonBooleanTest, SetJsonDataCaseInsensitive)
{
    auto json_bool1 = std::make_shared<JsonBoolean>();
    auto json_bool2 = std::make_shared<JsonBoolean>();
    auto json_bool3 = std::make_shared<JsonBoolean>();
    auto json_bool4 = std::make_shared<JsonBoolean>();

    EXPECT_TRUE(json_bool1->set_json_data("TRUE"));
    EXPECT_EQ(json_bool1->value, true);

    EXPECT_TRUE(json_bool2->set_json_data("FALSE"));
    EXPECT_EQ(json_bool2->value, false);

    EXPECT_TRUE(json_bool3->set_json_data("True"));
    EXPECT_EQ(json_bool3->value, true);

    EXPECT_TRUE(json_bool4->set_json_data("False"));
    EXPECT_EQ(json_bool4->value, false);
}

TEST_F(JsonBooleanTest, SetJsonDataWithInvalidValue)
{
    auto json_bool = std::make_shared<JsonBoolean>();
    EXPECT_FALSE(json_bool->set_json_data("invalid"));
    EXPECT_FALSE(json_bool->set_json_data("1"));
    EXPECT_FALSE(json_bool->set_json_data("0"));
    EXPECT_FALSE(json_bool->set_json_data(""));
    EXPECT_FALSE(json_bool->set_json_data("yes"));
    EXPECT_FALSE(json_bool->set_json_data("no"));
}

TEST_F(JsonBooleanTest, GetMethodThrowsException)
{
    auto json_bool = std::make_shared<JsonBoolean>(true);
    EXPECT_THROW(json_bool->get("any_key"), std::runtime_error);
}

TEST_F(JsonBooleanTest, StringifyConsistency)
{
    auto json_bool_true = std::make_shared<JsonBoolean>(true);
    auto json_bool_false = std::make_shared<JsonBoolean>(false);

    // Test multiple calls to stringify return consistent results
    EXPECT_EQ(json_bool_true->stringify(), "true");
    EXPECT_EQ(json_bool_true->stringify(), "true");

    EXPECT_EQ(json_bool_false->stringify(), "false");
    EXPECT_EQ(json_bool_false->stringify(), "false");
}

// Integration test - Test JsonBoolean within JsonObject
TEST_F(JsonBooleanTest, JsonBooleanInObject)
{
    auto obj = std::make_shared<JsonObject>();
    auto bool_true = std::make_shared<JsonBoolean>(true);
    auto bool_false = std::make_shared<JsonBoolean>(false);

    obj->insert("is_active", bool_true);
    obj->insert("is_deleted", bool_false);

    // Verify the values are stored correctly
    auto active_val = std::dynamic_pointer_cast<JsonBoolean>(obj->get("is_active"));
    auto deleted_val = std::dynamic_pointer_cast<JsonBoolean>(obj->get("is_deleted"));

    ASSERT_NE(active_val, nullptr);
    ASSERT_NE(deleted_val, nullptr);

    EXPECT_EQ(active_val->value, true);
    EXPECT_EQ(deleted_val->value, false);

    // Test stringify includes boolean values correctly
    std::string json_str = obj->stringify();
    EXPECT_NE(json_str.find("true"), std::string::npos);
    EXPECT_NE(json_str.find("false"), std::string::npos);
}

// Test JsonBoolean within JsonArray
TEST_F(JsonBooleanTest, JsonBooleanInArray)
{
    auto arr = std::make_shared<JsonArray>();
    arr->insert(std::make_shared<JsonBoolean>(true));
    arr->insert(std::make_shared<JsonBoolean>(false));
    arr->insert(std::make_shared<JsonBoolean>(true));

    EXPECT_EQ(arr->elements.size(), 3);

    // Verify each element
    auto elem1 = std::dynamic_pointer_cast<JsonBoolean>(arr->elements[0]);
    auto elem2 = std::dynamic_pointer_cast<JsonBoolean>(arr->elements[1]);
    auto elem3 = std::dynamic_pointer_cast<JsonBoolean>(arr->elements[2]);

    ASSERT_NE(elem1, nullptr);
    ASSERT_NE(elem2, nullptr);
    ASSERT_NE(elem3, nullptr);

    EXPECT_EQ(elem1->value, true);
    EXPECT_EQ(elem2->value, false);
    EXPECT_EQ(elem3->value, true);

    // Test stringify
    std::string arr_str = arr->stringify();
    EXPECT_EQ(arr_str, "[true,false,true]");
}

// Test edge cases
TEST_F(JsonBooleanTest, EdgeCases)
{
    auto json_bool = std::make_shared<JsonBoolean>();

    // Test with whitespace
    EXPECT_FALSE(json_bool->set_json_data(" true ")); // Should not accept whitespace
    EXPECT_FALSE(json_bool->set_json_data("true "));
    EXPECT_FALSE(json_bool->set_json_data(" true"));

    // Test partial matches
    EXPECT_FALSE(json_bool->set_json_data("tr"));
    EXPECT_FALSE(json_bool->set_json_data("truee"));
    EXPECT_FALSE(json_bool->set_json_data("fals"));
    EXPECT_FALSE(json_bool->set_json_data("falsee"));
}

// Test copy behavior
TEST_F(JsonBooleanTest, CopyBehavior)
{
    auto original = std::make_shared<JsonBoolean>(true);
    auto copy = std::make_shared<JsonBoolean>(original->value);

    EXPECT_EQ(original->value, copy->value);
    EXPECT_EQ(original->stringify(), copy->stringify());

    // Modify copy, original should remain unchanged
    copy->value = false;
    EXPECT_NE(original->value, copy->value);
    EXPECT_TRUE(original->value);
    EXPECT_FALSE(copy->value);
}
