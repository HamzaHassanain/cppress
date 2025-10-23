/**
 * @file http_parse_result.hpp
 * @brief Result structure for HTTP message parsing operations
 *
 * This file defines the data structure returned by http_request_parser
 * after processing incoming HTTP data. It indicates whether a complete
 * request has been parsed and provides access to all request components.
 *
 * Used internally by http_server to determine when to invoke user
 * callbacks with complete http_request and http_response objects.
 *
 * @note This is an internal implementation detail
 */

#pragma once

#include <map>
#include <string>

namespace cppress::http {

/**
 * @struct http_parse_result
 * @brief Result of HTTP message parsing operation
 *
 * Returned by http_request_parser::parse() to indicate parsing status
 * and provide access to parsed request components. When is_complete=true,
 * all fields contain valid data and can be used to construct http_request.
 *
 * When is_complete=false, the request is incomplete and additional data
 * is needed. The parser will continue accumulating data internally.
 */
struct http_parse_result {
    /// true if complete request received and parsed successfully
    bool is_complete;

    /// HTTP method (GET, POST, PUT, DELETE, etc.)
    std::string method;

    /// Request URI/path
    std::string uri;

    /// HTTP version (typically "HTTP/1.1")
    std::string http_version;

    /// Request headers (multimap allows duplicate header names)
    std::multimap<std::string, std::string> headers;

    /// Complete request body (empty for GET/HEAD requests)
    std::string body;

    /**
     * @brief Construct a parse result
     * @param complete Whether parsing is complete
     * @param method HTTP method
     * @param uri Request URI
     * @param version HTTP version
     * @param headers Request headers
     * @param body Request body
     */
    http_parse_result(bool complete, const std::string& method, const std::string& uri,
                      const std::string& version,
                      const std::multimap<std::string, std::string>& headers,
                      const std::string& body)
        : is_complete(complete),
          method(method),
          uri(uri),
          http_version(version),
          headers(headers),
          body(body) {}

    /**
     * @brief Check if parsing is incomplete
     * @return true if more data is needed
     */
    bool is_incomplete() const { return !is_complete; }

    /**
     * @brief Convert to human-readable string for debugging
     * @return Formatted string with all request components
     */
    std::string to_string() const {
        std::string result = "Complete: " + std::string(is_complete ? "true" : "false") + "\n";
        result += "Method: " + method + "\n";
        result += "URI: " + uri + "\n";
        result += "Version: " + http_version + "\n";
        result += "Headers:\n";
        for (const auto& header : headers) {
            result += "  " + header.first + ": " + header.second + "\n";
        }
        result += "Body: " + body + "\n";
        return result;
    }
};

}  // namespace cppress::http
