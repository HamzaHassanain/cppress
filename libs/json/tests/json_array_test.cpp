#include <gtest/gtest.h>
#include "../json-parser.hpp"
#include <memory>
#include <string>

using namespace hh_json;

class JsonArrayTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Set up test fixtures if needed
    }
};

TEST_F(JsonArrayTest, DefaultConstructor)
{
    auto arr = std::make_shared<JsonArray>();
    EXPECT_EQ(arr->elements.size(), 0);
    EXPECT_EQ(arr->stringify(), "[]");
}

TEST_F(JsonArrayTest, InsertAndAccess)
{
    auto arr = std::make_shared<JsonArray>();
    auto str_val = std::make_shared<JsonString>("test");

    arr->insert(str_val);
    EXPECT_EQ(arr->elements.size(), 1);

    auto retrieved = std::dynamic_pointer_cast<JsonString>(arr->elements[0]);
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->value, "test");
}

TEST_F(JsonArrayTest, InsertMultipleElements)
{
    auto arr = std::make_shared<JsonArray>();

    arr->insert(std::make_shared<JsonString>("first"));
    arr->insert(std::make_shared<JsonNumber>(42));
    arr->insert(std::make_shared<JsonBoolean>(true));

    EXPECT_EQ(arr->elements.size(), 3);

    auto str_elem = std::dynamic_pointer_cast<JsonString>(arr->elements[0]);
    auto num_elem = std::dynamic_pointer_cast<JsonNumber>(arr->elements[1]);
    auto bool_elem = std::dynamic_pointer_cast<JsonBoolean>(arr->elements[2]);

    ASSERT_NE(str_elem, nullptr);
    ASSERT_NE(num_elem, nullptr);
    ASSERT_NE(bool_elem, nullptr);

    EXPECT_EQ(str_elem->value, "first");
    EXPECT_DOUBLE_EQ(num_elem->value, 42.0);
    EXPECT_TRUE(bool_elem->value);
}

TEST_F(JsonArrayTest, StringifyEmptyArray)
{
    auto arr = std::make_shared<JsonArray>();
    EXPECT_EQ(arr->stringify(), "[]");
}

TEST_F(JsonArrayTest, StringifySimpleArray)
{
    auto arr = std::make_shared<JsonArray>();
    arr->insert(std::make_shared<JsonString>("first"));
    arr->insert(std::make_shared<JsonNumber>(42));
    arr->insert(std::make_shared<JsonBoolean>(true));

    std::string result = arr->stringify();
    EXPECT_EQ(result, "[\"first\",42,true]");
}

TEST_F(JsonArrayTest, StringifyNestedArray)
{
    auto outer_arr = std::make_shared<JsonArray>();
    auto inner_arr = std::make_shared<JsonArray>();

    inner_arr->insert(std::make_shared<JsonNumber>(1));
    inner_arr->insert(std::make_shared<JsonNumber>(2));

    outer_arr->insert(inner_arr);
    outer_arr->insert(std::make_shared<JsonString>("after_array"));

    std::string result = outer_arr->stringify();
    EXPECT_EQ(result, "[[1,2],\"after_array\"]");
}

TEST_F(JsonArrayTest, ArrayWithObjects)
{
    auto arr = std::make_shared<JsonArray>();
    auto obj1 = std::make_shared<JsonObject>();
    auto obj2 = std::make_shared<JsonObject>();

    obj1->insert("name", std::make_shared<JsonString>("Alice"));
    obj2->insert("name", std::make_shared<JsonString>("Bob"));

    arr->insert(obj1);
    arr->insert(obj2);

    EXPECT_EQ(arr->elements.size(), 2);

    auto retrieved_obj1 = std::dynamic_pointer_cast<JsonObject>(arr->elements[0]);
    auto retrieved_obj2 = std::dynamic_pointer_cast<JsonObject>(arr->elements[1]);

    ASSERT_NE(retrieved_obj1, nullptr);
    ASSERT_NE(retrieved_obj2, nullptr);

    auto name1 = std::dynamic_pointer_cast<JsonString>(retrieved_obj1->get("name"));
    auto name2 = std::dynamic_pointer_cast<JsonString>(retrieved_obj2->get("name"));

    ASSERT_NE(name1, nullptr);
    ASSERT_NE(name2, nullptr);

    EXPECT_EQ(name1->value, "Alice");
    EXPECT_EQ(name2->value, "Bob");
}

TEST_F(JsonArrayTest, GetMethodThrowsException)
{
    auto arr = std::make_shared<JsonArray>();
    arr->insert(std::make_shared<JsonString>("test"));

    // JsonArray doesn't support key-based access
    EXPECT_THROW(arr->get("any_key"), std::runtime_error);
}

TEST_F(JsonArrayTest, SetJsonData)
{
    auto arr = std::make_shared<JsonArray>();

    std::string json_data = R"([1, "two", false])";

    EXPECT_TRUE(arr->set_json_data(json_data));

    EXPECT_EQ(arr->elements.size(), 3);

    std::string bad_data = R"({"not": "an array"})";

    EXPECT_FALSE(arr->set_json_data(bad_data));
}

TEST_F(JsonArrayTest, LargeArray)
{
    auto arr = std::make_shared<JsonArray>();

    // Insert many elements
    for (int i = 0; i < 1000; ++i)
    {
        arr->insert(std::make_shared<JsonNumber>(i));
    }

    EXPECT_EQ(arr->elements.size(), 1000);

    // Check a few specific elements
    auto elem_500 = std::dynamic_pointer_cast<JsonNumber>(arr->elements[500]);
    ASSERT_NE(elem_500, nullptr);
    EXPECT_DOUBLE_EQ(elem_500->value, 500.0);
}

TEST_F(JsonArrayTest, MixedTypeArray)
{
    auto arr = std::make_shared<JsonArray>();

    // Mix different JSON types
    arr->insert(std::make_shared<JsonString>("string"));
    arr->insert(std::make_shared<JsonNumber>(123.45));
    arr->insert(std::make_shared<JsonBoolean>(false));
    arr->insert(std::make_shared<JsonObject>());
    arr->insert(std::make_shared<JsonArray>());

    EXPECT_EQ(arr->elements.size(), 5);

    // Verify types
    EXPECT_NE(std::dynamic_pointer_cast<JsonString>(arr->elements[0]), nullptr);
    EXPECT_NE(std::dynamic_pointer_cast<JsonNumber>(arr->elements[1]), nullptr);
    EXPECT_NE(std::dynamic_pointer_cast<JsonBoolean>(arr->elements[2]), nullptr);
    EXPECT_NE(std::dynamic_pointer_cast<JsonObject>(arr->elements[3]), nullptr);
    EXPECT_NE(std::dynamic_pointer_cast<JsonArray>(arr->elements[4]), nullptr);
}

TEST_F(JsonArrayTest, ArrayOfArrays)
{
    auto main_arr = std::make_shared<JsonArray>();

    for (int i = 0; i < 3; ++i)
    {
        auto sub_arr = std::make_shared<JsonArray>();
        for (int j = 0; j < 3; ++j)
        {
            sub_arr->insert(std::make_shared<JsonNumber>(i * 3 + j));
        }
        main_arr->insert(sub_arr);
    }

    EXPECT_EQ(main_arr->elements.size(), 3);

    // Check nested structure
    auto first_sub = std::dynamic_pointer_cast<JsonArray>(main_arr->elements[0]);
    ASSERT_NE(first_sub, nullptr);
    EXPECT_EQ(first_sub->elements.size(), 3);

    auto first_elem = std::dynamic_pointer_cast<JsonNumber>(first_sub->elements[0]);
    ASSERT_NE(first_elem, nullptr);
    EXPECT_DOUBLE_EQ(first_elem->value, 0.0);
}

TEST_F(JsonArrayTest, ArrayModification)
{
    auto arr = std::make_shared<JsonArray>();

    arr->insert(std::make_shared<JsonString>("original"));
    EXPECT_EQ(arr->elements.size(), 1);

    // Direct access and modification
    auto elem = std::dynamic_pointer_cast<JsonString>(arr->elements[0]);
    ASSERT_NE(elem, nullptr);
    elem->value = "modified";

    EXPECT_EQ(elem->value, "modified");
}

TEST_F(JsonArrayTest, EmptyElementsHandling)
{
    auto arr = std::make_shared<JsonArray>();

    // Insert empty string and zero number
    arr->insert(std::make_shared<JsonString>(""));
    arr->insert(std::make_shared<JsonNumber>(0));
    arr->insert(std::make_shared<JsonObject>());
    arr->insert(std::make_shared<JsonArray>());

    EXPECT_EQ(arr->elements.size(), 4);

    std::string result = arr->stringify();
    EXPECT_EQ(result, "[\"\",0,{},[]]");
}
