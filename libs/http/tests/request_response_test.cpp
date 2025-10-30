/**
 * @file request_response_test.cpp
 * @brief Unit tests for http_request and http_response new API functions
 */

#include <gtest/gtest.h>

#include "../includes.hpp"

using namespace cppress::http;

class RequestResponseAPITest : public ::testing::Test {
protected:
    std::function<void()> dummy_close = []() {};
    std::function<void(const std::string&)> dummy_send = [](const std::string&) {};
};

TEST_F(RequestResponseAPITest, GetPathWithoutQueryString) {
    std::multimap<std::string, std::string> headers;

    http_request req1("GET", "/search?q=test&page=2", "HTTP/1.1", headers, "", dummy_close);
    EXPECT_EQ(req1.get_path(), "/search");

    http_request req2("GET", "/api/users", "HTTP/1.1", headers, "", dummy_close);
    EXPECT_EQ(req2.get_path(), "/api/users");

    http_request req3("GET", "/home?", "HTTP/1.1", headers, "", dummy_close);
    EXPECT_EQ(req3.get_path(), "/home");

    http_request req4("GET", "/", "HTTP/1.1", headers, "", dummy_close);
    EXPECT_EQ(req4.get_path(), "/");
}

TEST_F(RequestResponseAPITest, GetQueryParamsParser) {
    std::multimap<std::string, std::string> headers;

    http_request req1("GET", "/search?q=test&page=2&limit=10", "HTTP/1.1", headers, "",
                      dummy_close);
    auto params1 = req1.get_query_params();
    EXPECT_EQ(params1.size(), 3);
    EXPECT_EQ(params1["q"], "test");
    EXPECT_EQ(params1["page"], "2");
    EXPECT_EQ(params1["limit"], "10");

    http_request req2("GET", "/search?q=hello+world&name=John+Doe", "HTTP/1.1", headers, "",
                      dummy_close);
    auto params2 = req2.get_query_params();
    EXPECT_EQ(params2["q"], "hello world");
    EXPECT_EQ(params2["name"], "John Doe");

    http_request req3("GET", "/search", "HTTP/1.1", headers, "", dummy_close);
    auto params3 = req3.get_query_params();
    EXPECT_TRUE(params3.empty());

    http_request req4("GET", "/search?q=test", "HTTP/1.1", headers, "", dummy_close);
    auto params4 = req4.get_query_params();
    EXPECT_EQ(params4.size(), 1);
    EXPECT_EQ(params4["q"], "test");

    http_request req5("GET", "/search?flag&q=test", "HTTP/1.1", headers, "", dummy_close);
    auto params5 = req5.get_query_params();
    EXPECT_EQ(params5["flag"], "");
    EXPECT_EQ(params5["q"], "test");
}

TEST_F(RequestResponseAPITest, GetSingleQueryParam) {
    std::multimap<std::string, std::string> headers;

    http_request req("GET", "/search?q=hello+world&page=2&limit=10", "HTTP/1.1", headers, "",
                     dummy_close);

    EXPECT_EQ(req.get_query_param("q"), "hello world");
    EXPECT_EQ(req.get_query_param("page"), "2");
    EXPECT_EQ(req.get_query_param("limit"), "10");
    EXPECT_EQ(req.get_query_param("nonexistent"), "");
}

TEST_F(RequestResponseAPITest, HasHeaderCheck) {
    std::multimap<std::string, std::string> headers;
    headers.insert({"CONTENT-TYPE", "application/json"});
    headers.insert({"AUTHORIZATION", "Bearer token123"});

    http_request req("POST", "/api/data", "HTTP/1.1", headers, "{}", dummy_close);

    EXPECT_TRUE(req.has_header("content-type"));
    EXPECT_TRUE(req.has_header("CONTENT-TYPE"));
    EXPECT_TRUE(req.has_header("Content-Type"));
    EXPECT_TRUE(req.has_header("authorization"));
    EXPECT_FALSE(req.has_header("Accept"));
    EXPECT_FALSE(req.has_header("X-Custom-Header"));
}

TEST_F(RequestResponseAPITest, ResponseConvenienceMethods) {
    std::multimap<std::string, std::string> headers;
    std::string sent_response;

    auto capture_send = [&sent_response](const std::string& msg) { sent_response = msg; };

    // Test json() method
    {
        http_response res("HTTP/1.1", headers, dummy_close, capture_send);
        res.json("{\"status\":\"ok\"}");

        EXPECT_NE(sent_response.find("HTTP/1.1 200 OK"), std::string::npos);
        EXPECT_NE(sent_response.find("CONTENT-TYPE: application/json"), std::string::npos);
        EXPECT_NE(sent_response.find("{\"status\":\"ok\"}"), std::string::npos);
        EXPECT_NE(sent_response.find("Content-Length:"), std::string::npos);
    }

    // Test html() method
    {
        sent_response.clear();
        http_response res("HTTP/1.1", headers, dummy_close, capture_send);
        res.html("<h1>Welcome</h1>", 201);

        EXPECT_NE(sent_response.find("HTTP/1.1 201 Created"), std::string::npos);
        EXPECT_NE(sent_response.find("CONTENT-TYPE: text/html"), std::string::npos);
        EXPECT_NE(sent_response.find("<h1>Welcome</h1>"), std::string::npos);
    }

    // Test text() method
    {
        sent_response.clear();
        http_response res("HTTP/1.1", headers, dummy_close, capture_send);
        res.text("Hello World", 404);

        EXPECT_NE(sent_response.find("HTTP/1.1 404 Not Found"), std::string::npos);
        EXPECT_NE(sent_response.find("CONTENT-TYPE: text/plain"), std::string::npos);
        EXPECT_NE(sent_response.find("Hello World"), std::string::npos);
    }
}

TEST_F(RequestResponseAPITest, ResponseHeaderManipulation) {
    std::multimap<std::string, std::string> headers;
    std::string sent_response;

    auto capture_send = [&sent_response](const std::string& msg) { sent_response = msg; };

    http_response res("HTTP/1.1", headers, dummy_close, capture_send);

    // Add headers
    res.add_header("X-Custom", "value1");
    res.add_header("X-Custom", "value2");
    EXPECT_TRUE(res.has_header("X-Custom"));

    // set_header should replace all values
    res.set_header("X-Custom", "new-value");
    res.set_body("test");
    res.send();

    // Count occurrences of X-Custom header (should be only 1)
    size_t pos = 0;
    int count = 0;
    while ((pos = sent_response.find("X-CUSTOM:", pos)) != std::string::npos) {
        count++;
        pos += 9;
    }
    EXPECT_EQ(count, 1);
    EXPECT_NE(sent_response.find("X-CUSTOM: new-value"), std::string::npos);
    EXPECT_EQ(sent_response.find("value1"), std::string::npos);
    EXPECT_EQ(sent_response.find("value2"), std::string::npos);

    // Test remove_header
    sent_response.clear();
    http_response res2("HTTP/1.1", headers, dummy_close, capture_send);
    res2.add_header("X-Test", "test");
    res2.remove_header("X-Test");
    res2.set_body("test");
    res2.send();

    EXPECT_EQ(sent_response.find("X-TEST:"), std::string::npos);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
