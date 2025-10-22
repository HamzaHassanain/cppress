#include <gtest/gtest.h>

#include "../includes.hpp"

class TestHtmlBuilder : public ::testing::Test {
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

TEST(TestHtmlBuilder, CreateSimpleElement) {
    auto elem = std::make_shared<hh_html_builder::element>("div");
    elem->set_text_content("Hello, World!");

    EXPECT_EQ(elem->to_string(), R"(<div>Hello, World!</div>)");
}
