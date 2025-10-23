/**
 * @file http_consts.hpp
 * @brief HTTP protocol constants and configuration parameters
 *
 * This file provides HTTP/1.1 standard constants including status codes,
 * common header names, protocol versions, and runtime configuration values.
 *
 * The config namespace contains mutable runtime settings that can be adjusted
 * before starting the server to customize size limits and timeout behavior.
 *
 * The consts namespace provides compile-time constants for HTTP protocol
 * elements, ensuring consistency and avoiding magic strings throughout the codebase.
 *
 * @example
 * @code
 * // Configure server limits before starting
 * cppress::http::config::MAX_BODY_SIZE = 5 * 1024 * 1024;  // 5 MB
 * cppress::http::config::TIMEOUT_MILLISECONDS = 3000;  // 3 seconds
 *
 * // Use constants in response handling
 * response.set_status(cppress::http::consts::HTTP_OK, "OK");
 * response.add_header(cppress::http::consts::HEADER_CONTENT_TYPE, "application/json");
 * @endcode
 */

#pragma once

#include <algorithm>
#include <chrono>
#include <string>
namespace cppress::http {

/**
 * @namespace cppress::http::config
 * @brief Runtime configuration parameters for HTTP server behavior
 *
 * These values can be modified before server startup to customize
 * size limits, timeouts, and connection handling behavior.
 *
 * @warning Changes to these values after server starts may not take effect
 */
namespace config {
/// Maximum size of HTTP headers (default: system-dependent)
extern size_t MAX_HEADER_SIZE;

/// Maximum size of HTTP request body (default: system-dependent)
extern size_t MAX_BODY_SIZE;

/// Maximum idle time before connection cleanup (default: system-dependent)
extern std::chrono::seconds MAX_IDLE_TIME_SECONDS;

/// Server socket listen backlog size (default: system-dependent)
extern int BACKLOG_SIZE;

/// Maximum number of file descriptors for epoll/select (default: system-dependent)
extern int MAX_FILE_DESCRIPTORS;

/// Timeout for epoll/select operations in milliseconds (default: system-dependent)
extern int TIMEOUT_MILLISECONDS;
}  // namespace config

/**
 * @namespace cppress::http::consts
 * @brief HTTP/1.1 protocol constants
 *
 * Provides compile-time constants for HTTP versions, status codes,
 * standard header names, and protocol delimiters.
 */
namespace consts {
/// HTTP protocol version 1.0
constexpr const char* HTTP_VERSION_1_0 = "HTTP/1.0";

/// HTTP protocol version 1.1
constexpr const char* HTTP_VERSION_1_1 = "HTTP/1.1";

/// 200 - Request succeeded
constexpr int HTTP_OK = 200;

/// 201 - Resource created successfully
constexpr int HTTP_CREATED = 201;

/// 204 - No content to return
constexpr int HTTP_NO_CONTENT = 204;

/// 400 - Malformed request
constexpr int HTTP_BAD_REQUEST = 400;

/// 401 - Authentication required
constexpr int HTTP_UNAUTHORIZED = 401;

/// 403 - Access denied
constexpr int HTTP_FORBIDDEN = 403;

/// 404 - Resource not found
constexpr int HTTP_NOT_FOUND = 404;

/// 500 - Server error
constexpr int HTTP_INTERNAL_SERVER_ERROR = 500;

/// Content-Type header name
constexpr const char* HEADER_CONTENT_TYPE = "Content-Type";

/// Content-Length header name
constexpr const char* HEADER_CONTENT_LENGTH = "Content-Length";

/// Connection header name
constexpr const char* HEADER_CONNECTION = "Connection";

/// Host header name
constexpr const char* HEADER_HOST = "Host";

/// User-Agent header name
constexpr const char* HEADER_USER_AGENT = "User-Agent";

/// Accept header name
constexpr const char* HEADER_ACCEPT = "Accept";

/// Authorization header name
constexpr const char* HEADER_AUTHORIZATION = "Authorization";

/// Referer header name
constexpr const char* HEADER_REFERER = "Referer";

/// Cookie header name
constexpr const char* HEADER_COOKIE = "Cookie";

/// If-Modified-Since header name
constexpr const char* HEADER_IF_MODIFIED_SINCE = "If-Modified-Since";

/// If-None-Match header name
constexpr const char* HEADER_IF_NONE_MATCH = "If-None-Match";

/// Expect header name (e.g., "100-continue")
constexpr const char* HEADER_EXPECT = "Expect";

/// HTTP line ending sequence
constexpr const char* CRLF = "\r\n";

/// HTTP header/body separator
constexpr const char* DOUBLE_CRLF = "\r\n\r\n";
}  // namespace consts
}  // namespace cppress::http