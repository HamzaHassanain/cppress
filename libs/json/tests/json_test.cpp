#include <gtest/gtest.h>

#include "../includes.hpp"

class TestJsonBuilder : public ::testing::Test {
protected:
    void SetUp() override {
        // Code here will be called immediately after the constructor (right
        // before each test).
    }

    void TearDown() override {
        // Code here will be called immediately after each test (right
        // before the destructor).
    }
};

TEST(TestJsonBuilder, CreateSimpleObject) {
    auto obj = std::make_shared<hh_json::JsonObject>();

    EXPECT_EQ(obj->stringify(), R"({})");
}