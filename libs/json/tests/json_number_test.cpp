#include <gtest/gtest.h>
#include "../json-parser.hpp"
#include <memory>
#include <string>
#include <cmath>

using namespace hh_json;

class JsonNumberTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Set up test fixtures if needed
    }
};

TEST_F(JsonNumberTest, ConstructorWithInteger)
{
    auto json_num = std::make_shared<JsonNumber>(42);
    EXPECT_DOUBLE_EQ(json_num->value, 42.0);
    EXPECT_EQ(json_num->stringify(), "42");
}

TEST_F(JsonNumberTest, ConstructorWithDouble)
{
    auto json_num = std::make_shared<JsonNumber>(3.14159);
    EXPECT_DOUBLE_EQ(json_num->value, 3.14159);
}

TEST_F(JsonNumberTest, ConstructorWithNegativeNumber)
{
    auto json_num = std::make_shared<JsonNumber>(-123.456);
    EXPECT_DOUBLE_EQ(json_num->value, -123.456);
}

TEST_F(JsonNumberTest, ConstructorWithZero)
{
    auto json_num = std::make_shared<JsonNumber>(0);
    EXPECT_DOUBLE_EQ(json_num->value, 0.0);
    EXPECT_EQ(json_num->stringify(), "0");
}

TEST_F(JsonNumberTest, DefaultConstructor)
{
    auto json_num = std::make_shared<JsonNumber>();
    EXPECT_DOUBLE_EQ(json_num->value, 0.0);
    EXPECT_EQ(json_num->stringify(), "0");
}

TEST_F(JsonNumberTest, SetJsonDataWithValidNumber)
{
    auto json_num = std::make_shared<JsonNumber>();

    EXPECT_TRUE(json_num->set_json_data("123.456"));
    EXPECT_DOUBLE_EQ(json_num->value, 123.456);

    EXPECT_TRUE(json_num->set_json_data("-789"));
    EXPECT_DOUBLE_EQ(json_num->value, -789.0);

    EXPECT_TRUE(json_num->set_json_data("0"));
    EXPECT_DOUBLE_EQ(json_num->value, 0.0);

    EXPECT_TRUE(json_num->set_json_data("0.0"));
    EXPECT_DOUBLE_EQ(json_num->value, 0.0);
}

TEST_F(JsonNumberTest, SetJsonDataWithInvalidNumber)
{
    auto json_num = std::make_shared<JsonNumber>();

    EXPECT_FALSE(json_num->set_json_data("not_a_number"));
    EXPECT_FALSE(json_num->set_json_data(""));
    EXPECT_FALSE(json_num->set_json_data("12.34.56"));
    EXPECT_FALSE(json_num->set_json_data("12a34"));
    EXPECT_FALSE(json_num->set_json_data("++123"));
    EXPECT_FALSE(json_num->set_json_data("--123"));
}

TEST_F(JsonNumberTest, SetJsonDataWithScientificNotation)
{
    auto json_num = std::make_shared<JsonNumber>();

    EXPECT_TRUE(json_num->set_json_data("1.23e10"));
    EXPECT_DOUBLE_EQ(json_num->value, 1.23e10);

    EXPECT_TRUE(json_num->set_json_data("5e-3"));
    EXPECT_DOUBLE_EQ(json_num->value, 5e-3);

    EXPECT_TRUE(json_num->set_json_data("-2.5E+2"));
    EXPECT_DOUBLE_EQ(json_num->value, -2.5E+2);
}

TEST_F(JsonNumberTest, LargeNumbers)
{
    auto json_num = std::make_shared<JsonNumber>();

    EXPECT_TRUE(json_num->set_json_data("1234567890123456"));
    EXPECT_DOUBLE_EQ(json_num->value, 1234567890123456.0);

    EXPECT_TRUE(json_num->set_json_data("-9876543210.123456"));
    EXPECT_DOUBLE_EQ(json_num->value, -9876543210.123456);
}

TEST_F(JsonNumberTest, EdgeCaseNumbers)
{
    auto json_num = std::make_shared<JsonNumber>();

    // Very small decimal
    EXPECT_TRUE(json_num->set_json_data("0.000001"));
    EXPECT_DOUBLE_EQ(json_num->value, 0.000001);

    // Just a decimal point (should be invalid)
    EXPECT_FALSE(json_num->set_json_data("."));
    EXPECT_FALSE(json_num->set_json_data("-."));

    // Leading zeros (should be valid)
    EXPECT_TRUE(json_num->set_json_data("000123"));
    EXPECT_DOUBLE_EQ(json_num->value, 123.0);
}

TEST_F(JsonNumberTest, GetMethodThrowsException)
{
    auto json_num = std::make_shared<JsonNumber>(42.0);
    EXPECT_THROW(json_num->get("any_key"), std::runtime_error);
}

TEST_F(JsonNumberTest, StringifyIntegerFormat)
{
    auto json_num1 = std::make_shared<JsonNumber>(42.0);
    EXPECT_EQ(json_num1->stringify(), "42");

    auto json_num2 = std::make_shared<JsonNumber>(0.0);
    EXPECT_EQ(json_num2->stringify(), "0");

    auto json_num3 = std::make_shared<JsonNumber>(-5.0);
    EXPECT_EQ(json_num3->stringify(), "-5");
}

TEST_F(JsonNumberTest, StringifyFloatFormat)
{
    auto json_num1 = std::make_shared<JsonNumber>(3.14);
    std::string result1 = json_num1->stringify();
    EXPECT_NE(result1.find("3.14"), std::string::npos);

    auto json_num2 = std::make_shared<JsonNumber>(-123.456);
    std::string result2 = json_num2->stringify();
    EXPECT_NE(result2.find("-123.456"), std::string::npos);
}

TEST_F(JsonNumberTest, StringifyConsistency)
{
    auto json_num = std::make_shared<JsonNumber>(42.42);

    // Multiple calls should return the same result
    std::string result1 = json_num->stringify();
    std::string result2 = json_num->stringify();
    EXPECT_EQ(result1, result2);
}

// Integration tests
TEST_F(JsonNumberTest, JsonNumberInObject)
{
    auto obj = std::make_shared<JsonObject>();
    auto num1 = std::make_shared<JsonNumber>(123);
    auto num2 = std::make_shared<JsonNumber>(45.67);

    obj->insert("integer", num1);
    obj->insert("float", num2);

    auto retrieved1 = std::dynamic_pointer_cast<JsonNumber>(obj->get("integer"));
    auto retrieved2 = std::dynamic_pointer_cast<JsonNumber>(obj->get("float"));

    ASSERT_NE(retrieved1, nullptr);
    ASSERT_NE(retrieved2, nullptr);

    EXPECT_DOUBLE_EQ(retrieved1->value, 123.0);
    EXPECT_DOUBLE_EQ(retrieved2->value, 45.67);
}

TEST_F(JsonNumberTest, JsonNumberInArray)
{
    auto arr = std::make_shared<JsonArray>();
    arr->insert(std::make_shared<JsonNumber>(1));
    arr->insert(std::make_shared<JsonNumber>(2.5));
    arr->insert(std::make_shared<JsonNumber>(-3));

    EXPECT_EQ(arr->elements.size(), 3);

    auto elem1 = std::dynamic_pointer_cast<JsonNumber>(arr->elements[0]);
    auto elem2 = std::dynamic_pointer_cast<JsonNumber>(arr->elements[1]);
    auto elem3 = std::dynamic_pointer_cast<JsonNumber>(arr->elements[2]);

    ASSERT_NE(elem1, nullptr);
    ASSERT_NE(elem2, nullptr);
    ASSERT_NE(elem3, nullptr);

    EXPECT_DOUBLE_EQ(elem1->value, 1.0);
    EXPECT_DOUBLE_EQ(elem2->value, 2.5);
    EXPECT_DOUBLE_EQ(elem3->value, -3.0);
}

TEST_F(JsonNumberTest, MathematicalOperations)
{
    auto json_num = std::make_shared<JsonNumber>(10.5);

    // Test that we can modify the value directly
    json_num->value += 5.5;
    EXPECT_DOUBLE_EQ(json_num->value, 16.0);

    json_num->value *= 2;
    EXPECT_DOUBLE_EQ(json_num->value, 32.0);

    json_num->value /= 4;
    EXPECT_DOUBLE_EQ(json_num->value, 8.0);
}

TEST_F(JsonNumberTest, SpecialFloatValues)
{
    auto json_num = std::make_shared<JsonNumber>();

    // Test infinity (behavior may vary)
    json_num->value = std::numeric_limits<double>::infinity();
    EXPECT_TRUE(std::isinf(json_num->value));

    // Test NaN (behavior may vary)
    json_num->value = std::numeric_limits<double>::quiet_NaN();
    EXPECT_TRUE(std::isnan(json_num->value));
}

TEST_F(JsonNumberTest, PrecisionHandling)
{
    auto json_num = std::make_shared<JsonNumber>();

    // Test very precise decimal
    EXPECT_TRUE(json_num->set_json_data("1.2345678901234567890"));
    // Note: precision may be limited by double precision
    EXPECT_NEAR(json_num->value, 1.2345678901234567890, 1e-15);
}
