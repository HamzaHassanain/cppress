#include "../includes/http_request_parser.hpp"

#include <gtest/gtest.h>

#include "sockets/includes.hpp"

using namespace cppress::http;

std::shared_ptr<cppress::sockets::connection> make_mock_connection() {
    return std::make_shared<cppress::sockets::connection>();
}

TEST(HttpRequestParserTest, ParseCompleteGetRequest) {
    http_request_parser parser;
    auto conn = make_mock_connection();

    std::string request =
        "GET /index.html HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "\r\n";

    auto result = parser.parse(conn, cppress::sockets::data_buffer(request));

    EXPECT_TRUE(result.is_complete);
    EXPECT_EQ(result.method, "GET");
    EXPECT_EQ(result.uri, "/index.html");
    EXPECT_EQ(result.http_version, "HTTP/1.1");
    EXPECT_EQ(result.headers.count("HOST"), 1);
}

TEST(HttpRequestParserTest, ValidHTMLBodySmall) {
    http_request_parser parser;
    auto conn = make_mock_connection();

    std::string html = "<html><body><h1>Test</h1></body></html>";
    std::string request =
        "POST /submit HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: " +
        std::to_string(html.length()) +
        "\r\n"
        "\r\n" +
        html;

    auto result = parser.parse(conn, cppress::sockets::data_buffer(request));

    EXPECT_TRUE(result.is_complete);
    EXPECT_EQ(result.body, html);
    EXPECT_EQ(result.method, "POST");
}

TEST(HttpRequestParserTest, ValidHTMLBodySmall2) {
    http_request_parser parser;
    auto conn = make_mock_connection();

    std::string html = "<!DOCTYPE html><html><head><title>Page</title></head></html>";
    std::string request =
        "PUT /page HTTP/1.1\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: " +
        std::to_string(html.length()) +
        "\r\n"
        "\r\n" +
        html;

    auto result = parser.parse(conn, cppress::sockets::data_buffer(request));

    EXPECT_TRUE(result.is_complete);
    EXPECT_EQ(result.body, html);
}

TEST(HttpRequestParserTest, ValidHTMLBodyLarge) {
    http_request_parser parser;
    auto conn = make_mock_connection();

    std::string html =
        "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Large "
        "Page</title><style>body{margin:0;padding:20px;font-family:Arial}</style></"
        "head><body><header><h1>Welcome to Our Website</h1><nav><ul><li><a "
        "href=\"#home\">Home</a></li><li><a href=\"#about\">About</a></li><li><a "
        "href=\"#contact\">Contact</a></li></ul></nav></header><main><section><h2>Main "
        "Content</h2><p>This is a paragraph with lots of text content to make the HTML body larger "
        "for testing purposes. We need to ensure the parser can handle substantial HTML "
        "documents.</p><article><h3>Article Title</h3><p>Article content goes here with more "
        "text.</p></article></section></main><footer><p>&copy; 2025 Test "
        "Company</p></footer></body></html>";
    std::string request =
        "POST /document HTTP/1.1\r\n"
        "Host: test.example.com\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: " +
        std::to_string(html.length()) +
        "\r\n"
        "\r\n" +
        html;

    auto result = parser.parse(conn, cppress::sockets::data_buffer(request));

    EXPECT_TRUE(result.is_complete);
    EXPECT_EQ(result.body, html);
    EXPECT_EQ(result.method, "POST");
}

TEST(HttpRequestParserTest, ValidJSONBodySmall) {
    http_request_parser parser;
    auto conn = make_mock_connection();

    std::string json = "{\"name\":\"John\",\"age\":30}";
    std::string request =
        "POST /api/users HTTP/1.1\r\n"
        "Host: api.example.com\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: " +
        std::to_string(json.length()) +
        "\r\n"
        "\r\n" +
        json;

    auto result = parser.parse(conn, cppress::sockets::data_buffer(request));

    EXPECT_TRUE(result.is_complete);
    EXPECT_EQ(result.body, json);
    EXPECT_EQ(result.method, "POST");
}

TEST(HttpRequestParserTest, ValidJSONBodyLarge) {
    http_request_parser parser;
    auto conn = make_mock_connection();

    std::string json =
        "{\"users\":[{\"id\":1,\"name\":\"John "
        "Doe\",\"email\":\"john@example.com\",\"roles\":[\"admin\",\"user\"],\"settings\":{"
        "\"theme\":\"dark\",\"notifications\":true,\"language\":\"en\"}},{\"id\":2,\"name\":\"Jane "
        "Smith\",\"email\":\"jane@example.com\",\"roles\":[\"user\"],\"settings\":{\"theme\":"
        "\"light\",\"notifications\":false,\"language\":\"fr\"}},{\"id\":3,\"name\":\"Bob "
        "Johnson\",\"email\":\"bob@example.com\",\"roles\":[\"moderator\",\"user\"],\"settings\":{"
        "\"theme\":\"auto\",\"notifications\":true,\"language\":\"es\"}}],\"metadata\":{\"total\":"
        "3,\"page\":1,\"perPage\":10,\"timestamp\":\"2025-10-24T10:30:00Z\"}}";
    std::string request =
        "POST /api/users/bulk HTTP/1.1\r\n"
        "Host: api.example.com\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: " +
        std::to_string(json.length()) +
        "\r\n"
        "\r\n" +
        json;

    auto result = parser.parse(conn, cppress::sockets::data_buffer(request));

    EXPECT_TRUE(result.is_complete);
    EXPECT_EQ(result.body, json);
}

TEST(HttpRequestParserTest, InvalidHTMLBodySmall) {
    http_request_parser parser;
    auto conn = make_mock_connection();

    std::string html = "<html><body><h1>Unclosed header</body></html>";
    std::string request =
        "POST /submit HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: " +
        std::to_string(html.length()) +
        "\r\n"
        "\r\n" +
        html;

    auto result = parser.parse(conn, cppress::sockets::data_buffer(request));

    EXPECT_TRUE(result.is_complete);
    EXPECT_EQ(result.body, html);
}

TEST(HttpRequestParserTest, InvalidHTMLBodyLarge) {
    http_request_parser parser;
    auto conn = make_mock_connection();

    std::string html =
        "<html><head><title>Malformed</title><body><div class=\"main\"><section><h1>Header<p>This "
        "paragraph tag is not closed properly<div><span>Nested elements without proper "
        "closure<article><h2>Subheader</h3><ul><li>Item 1<li>Item 2<li>Item "
        "3</ol><table><tr><td>Cell 1<td>Cell 2</tr><tr><td>Cell 3</table></div></section></body>";
    std::string request =
        "POST /malformed HTTP/1.1\r\n"
        "Host: test.com\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: " +
        std::to_string(html.length()) +
        "\r\n"
        "\r\n" +
        html;

    auto result = parser.parse(conn, cppress::sockets::data_buffer(request));

    EXPECT_TRUE(result.is_complete);
    EXPECT_EQ(result.body, html);
}

TEST(HttpRequestParserTest, InvalidJSONBodySmall) {
    http_request_parser parser;
    auto conn = make_mock_connection();

    std::string json = "{\"name\":\"John\",\"age\":30";
    std::string request =
        "POST /api/users HTTP/1.1\r\n"
        "Host: api.example.com\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: " +
        std::to_string(json.length()) +
        "\r\n"
        "\r\n" +
        json;

    auto result = parser.parse(conn, cppress::sockets::data_buffer(request));

    EXPECT_TRUE(result.is_complete);
    EXPECT_EQ(result.body, json);
}

TEST(HttpRequestParserTest, InvalidJSONBodyLarge) {
    http_request_parser parser;
    auto conn = make_mock_connection();

    std::string json =
        "{\"users\":[{\"id\":1,\"name\":\"John "
        "Doe\",\"email\":\"john@example.com\",\"roles\":[\"admin\",\"user\"],\"settings\":{"
        "\"theme\":\"dark\",\"notifications\":true,\"language\":\"en\"}},{\"id\":2,\"name\":\"Jane "
        "Smith\",\"email\":\"jane@example.com\",\"roles\":[\"user\"],\"settings\":{\"theme\":"
        "\"light\",\"notifications\":false,\"language\":\"fr\"}},\"metadata\":{\"total\":3,"
        "\"page\":1,\"perPage\":10,\"timestamp\":\"2025-10-24T10:30:00Z\"";
    std::string request =
        "POST /api/users HTTP/1.1\r\n"
        "Host: api.example.com\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: " +
        std::to_string(json.length()) +
        "\r\n"
        "\r\n" +
        json;

    auto result = parser.parse(conn, cppress::sockets::data_buffer(request));

    EXPECT_TRUE(result.is_complete);
    EXPECT_EQ(result.body, json);
}

TEST(HttpRequestParserTest, MultipleHeadersSmall) {
    http_request_parser parser;
    auto conn = make_mock_connection();

    std::string request =
        "GET /resource HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "User-Agent: TestClient/1.0\r\n"
        "Accept: */*\r\n"
        "Connection: keep-alive\r\n"
        "\r\n";

    auto result = parser.parse(conn, cppress::sockets::data_buffer(request));

    EXPECT_TRUE(result.is_complete);
    EXPECT_EQ(result.headers.count("HOST"), 1);
    EXPECT_EQ(result.headers.count("USER-AGENT"), 1);
    EXPECT_EQ(result.headers.count("ACCEPT"), 1);
    EXPECT_EQ(result.headers.count("CONNECTION"), 1);
}

TEST(HttpRequestParserTest, MultipleHeadersLarge) {
    http_request_parser parser;
    auto conn = make_mock_connection();

    std::string request =
        "GET /page HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
        "Accept-Language: en-US,en;q=0.5\r\n"
        "Accept-Encoding: gzip, deflate, br\r\n"
        "Connection: keep-alive\r\n"
        "Cache-Control: max-age=0\r\n"
        "Upgrade-Insecure-Requests: 1\r\n"
        "Cookie: session=xyz789; user_id=12345\r\n"
        "X-Forwarded-For: 192.168.1.1\r\n"
        "X-Real-IP: 10.0.0.1\r\n"
        "X-Custom-Header: custom-value\r\n"
        "\r\n";

    auto result = parser.parse(conn, cppress::sockets::data_buffer(request));

    EXPECT_TRUE(result.is_complete);
    EXPECT_GE(result.headers.size(), 10);
    EXPECT_EQ(result.headers.count("HOST"), 1);
    EXPECT_EQ(result.headers.count("USER-AGENT"), 1);
    EXPECT_EQ(result.headers.count("ACCEPT"), 1);
    EXPECT_EQ(result.headers.count("COOKIE"), 1);
}

TEST(HttpRequestParserTest, MultipleHeadersLarge2) {
    http_request_parser parser;
    auto conn = make_mock_connection();

    std::string request =
        "POST /upload HTTP/1.1\r\n"
        "Host: upload.example.com\r\n"
        "User-Agent: FileUploader/2.0\r\n"
        "Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
        "Content-Length: 0\r\n"
        "Accept: application/json, text/plain, */*\r\n"
        "Accept-Language: en-US,en;q=0.9\r\n"
        "Accept-Encoding: gzip, deflate, br\r\n"
        "Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9\r\n"
        "Origin: https://example.com\r\n"
        "Referer: https://example.com/upload\r\n"
        "X-Requested-With: XMLHttpRequest\r\n"
        "X-API-Version: 2.0\r\n"
        "X-Client-ID: client-12345\r\n"
        "\r\n";

    auto result = parser.parse(conn, cppress::sockets::data_buffer(request));

    EXPECT_TRUE(result.is_complete);
    EXPECT_GE(result.headers.size(), 10);
    EXPECT_EQ(result.headers.count("AUTHORIZATION"), 1);
    EXPECT_EQ(result.headers.count("ORIGIN"), 1);
}

TEST(HttpRequestParserTest, HeadersWithHTMLSmall) {
    http_request_parser parser;
    auto conn = make_mock_connection();

    std::string html = "<html><body>Test</body></html>";
    std::string request =
        "POST /page HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Type: text/html\r\n"
        "User-Agent: Browser/1.0\r\n"
        "Content-Length: " +
        std::to_string(html.length()) +
        "\r\n"
        "\r\n" +
        html;

    auto result = parser.parse(conn, cppress::sockets::data_buffer(request));

    EXPECT_TRUE(result.is_complete);
    EXPECT_EQ(result.body, html);
    EXPECT_EQ(result.headers.count("HOST"), 1);
    EXPECT_EQ(result.headers.count("CONTENT-TYPE"), 1);
}

TEST(HttpRequestParserTest, HeadersWithHTMLLarge) {
    http_request_parser parser;
    auto conn = make_mock_connection();

    std::string html =
        "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Complex "
        "Document</"
        "title><style>body{font-family:Arial;margin:0;padding:20px}header{background:#333;color:"
        "white;padding:10px}</style></head><body><header><h1>Main Header</h1><nav><ul><li><a "
        "href=\"/home\">Home</a></li><li><a "
        "href=\"/about\">About</a></li></ul></nav></header><main><section "
        "class=\"content\"><h2>Section Title</h2><p>This is a longer HTML document with multiple "
        "elements and nested structures.</p><div class=\"box\"><article><h3>Article</h3><p>Article "
        "content goes here.</p></article></div></section></main></body></html>";
    std::string request =
        "POST /document HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "User-Agent: Mozilla/5.0\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Accept: text/html,application/xhtml+xml\r\n"
        "Accept-Language: en-US,en;q=0.9\r\n"
        "Cookie: session_id=abc123\r\n"
        "Content-Length: " +
        std::to_string(html.length()) +
        "\r\n"
        "\r\n" +
        html;

    auto result = parser.parse(conn, cppress::sockets::data_buffer(request));

    EXPECT_TRUE(result.is_complete);
    EXPECT_EQ(result.body, html);
    EXPECT_GE(result.headers.size(), 6);
    EXPECT_EQ(result.headers.count("HOST"), 1);
}

TEST(HttpRequestParserTest, HeadersWithJSONSmall) {
    http_request_parser parser;
    auto conn = make_mock_connection();

    std::string json = "{\"id\":1,\"name\":\"Test\"}";
    std::string request =
        "POST /api/resource HTTP/1.1\r\n"
        "Host: api.example.com\r\n"
        "Content-Type: application/json\r\n"
        "Authorization: Bearer abc123\r\n"
        "Content-Length: " +
        std::to_string(json.length()) +
        "\r\n"
        "\r\n" +
        json;

    auto result = parser.parse(conn, cppress::sockets::data_buffer(request));

    EXPECT_TRUE(result.is_complete);
    EXPECT_EQ(result.body, json);
    EXPECT_EQ(result.headers.count("HOST"), 1);
    EXPECT_EQ(result.headers.count("AUTHORIZATION"), 1);
}

TEST(HttpRequestParserTest, HeadersWithJSONSmall2) {
    http_request_parser parser;
    auto conn = make_mock_connection();

    std::string json = "{\"status\":\"ok\"}";
    std::string request =
        "PUT /api/status HTTP/1.1\r\n"
        "Host: status.example.com\r\n"
        "Content-Type: application/json; charset=utf-8\r\n"
        "X-API-Key: key-xyz\r\n"
        "Content-Length: " +
        std::to_string(json.length()) +
        "\r\n"
        "\r\n" +
        json;

    auto result = parser.parse(conn, cppress::sockets::data_buffer(request));

    EXPECT_TRUE(result.is_complete);
    EXPECT_EQ(result.body, json);
    EXPECT_GE(result.headers.size(), 3);
}

TEST(HttpRequestParserTest, HeadersWithJSONLarge) {
    http_request_parser parser;
    auto conn = make_mock_connection();

    std::string json =
        "{\"transaction\":{\"id\":\"TXN-98765\",\"timestamp\":\"2025-10-24T14:30:00Z\",\"amount\":"
        "1500.50,\"currency\":\"USD\",\"customer\":{\"id\":\"CUST-123\",\"name\":\"John "
        "Doe\",\"email\":\"john@example.com\",\"address\":{\"street\":\"123 Main "
        "St\",\"city\":\"New "
        "York\",\"state\":\"NY\",\"zip\":\"10001\"}},\"items\":[{\"id\":\"ITEM-1\",\"name\":"
        "\"Product A\",\"quantity\":2,\"price\":500.00},{\"id\":\"ITEM-2\",\"name\":\"Product "
        "B\",\"quantity\":1,\"price\":500.50}],\"payment\":{\"method\":\"credit_card\",\"last4\":"
        "\"1234\",\"status\":\"completed\"}}}";
    std::string request =
        "POST /api/transactions HTTP/1.1\r\n"
        "Host: payment.example.com\r\n"
        "User-Agent: PaymentClient/1.5\r\n"
        "Content-Type: application/json\r\n"
        "Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9\r\n"
        "Accept: application/json\r\n"
        "X-API-Version: 2.0\r\n"
        "X-Request-ID: req-12345\r\n"
        "Content-Length: " +
        std::to_string(json.length()) +
        "\r\n"
        "\r\n" +
        json;

    auto result = parser.parse(conn, cppress::sockets::data_buffer(request));

    EXPECT_TRUE(result.is_complete);
    EXPECT_EQ(result.body, json);
    EXPECT_GE(result.headers.size(), 7);
    EXPECT_EQ(result.headers.count("AUTHORIZATION"), 1);
}

TEST(HttpRequestParserTest, InvalidHeadersSmall) {
    http_request_parser parser;
    auto conn = make_mock_connection();

    std::string request =
        "GET /test HTTP/1.1\r\n"
        "HostWithoutColon example.com\r\n"
        "\r\n";

    auto result = parser.parse(conn, cppress::sockets::data_buffer(request));

    EXPECT_TRUE(result.is_complete);
}

TEST(HttpRequestParserTest, InvalidHeadersSmall2) {
    http_request_parser parser;
    auto conn = make_mock_connection();

    std::string request =
        "POST /api HTTP/1.1\r\n"
        "Content-Type\r\n"
        "Content-Length: 0\r\n"
        "\r\n";

    auto result = parser.parse(conn, cppress::sockets::data_buffer(request));
    EXPECT_TRUE(result.is_complete);
}

TEST(HttpRequestParserTest, InvalidHeadersLarge) {
    http_request_parser parser;
    auto conn = make_mock_connection();

    std::string request =
        "GET /resource HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "User-Agent Mozilla/5.0\r\n"
        "Accept: text/html\r\n"
        "Accept-Language en-US\r\n"
        "Accept-Encoding: gzip\r\n"
        "Connection keep-alive\r\n"
        "Cache-Control: no-cache\r\n"
        "\r\n";

    auto result = parser.parse(conn, cppress::sockets::data_buffer(request));

    EXPECT_TRUE(result.is_complete);
}

TEST(HttpRequestParserTest, ShortBodyWaitUntillComplete) {
    http_request_parser parser;
    auto conn = make_mock_connection();

    std::string request =
        "POST /data HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: 100\r\n"
        "\r\n"
        "short";

    auto result = parser.parse(conn, cppress::sockets::data_buffer(request));

    EXPECT_FALSE(result.is_complete);
}

TEST(HttpRequestParserTest, InvalidBodyLarge) {
    http_request_parser parser;
    auto conn = make_mock_connection();

    std::string actualBody(1000, 'A');  // 1000 'A's

    std::string request =
        "POST /upload HTTP/1.1\r\n"
        "Host: upload.example.com\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 500\r\n"
        "\r\n" +
        actualBody;

    auto result = parser.parse(conn, cppress::sockets::data_buffer(request));
    auto str = result.to_string();
    EXPECT_TRUE(str.find("BAD_CONTENT_TOO_LARGE") != std::string::npos);
    EXPECT_TRUE(result.is_complete);
}

TEST(HttpRequestParserTest, InvalidRequestSmall) {
    http_request_parser parser;
    auto conn = make_mock_connection();

    std::string request = "INVALID REQUEST\r\n\r\n";

    auto result = parser.parse(conn, cppress::sockets::data_buffer(request));

    EXPECT_TRUE(result.is_complete);
}

TEST(HttpRequestParserTest, InvalidRequestLarge) {
    http_request_parser parser;
    auto conn = make_mock_connection();

    std::string request =
        "CUSTOMMETHOD /path/to/resource HTTP/2.5\r\n"
        "Host: invalid.example.com\r\n"
        "Strange-Header-Without-Value\r\n"
        "Another-Malformed-Line\r\n"
        "Content-Type application/json\r\n"
        "X-Custom: value\r\n"
        "\r\n"
        "body content that may or may not be valid";

    auto result = parser.parse(conn, cppress::sockets::data_buffer(request));

    EXPECT_TRUE(result.is_complete);
}
