/**
 * @file utilities.hpp
 * @brief Utility functions for web request processing
 *
 * This file provides utility functions for common web operations such as
 * URL parsing, path matching, parameter extraction, and security validation.
 *
 * @section utility_functions Key Functions
 *
 * @subsection url_parsing URL Parsing
 * @li get_path() - Extract path from URI (without query string)
 * @li get_query_parameters() - Parse query string into key-value pairs
 * @li get_path_params() - Extract parameter names from route expressions
 *
 * @subsection route_matching Route Matching
 * @li match_path() - Match route patterns against request paths
 *     Supports named parameters (e.g., /users/:id)
 * @li is_uri_static() - Check if URI points to static resource
 *
 * @subsection security Security
 * @li body_has_malicious_content() - Detect XSS, SQL injection, command injection
 *
 * @section utilities_usage Usage Example
 * @code{.cpp}
 * // Parse query parameters
 * auto params = get_query_parameters("/search?q=test&page=1");
 * // params: {{"q", "test"}, {"page", "1"}}
 *
 * // Match route pattern
 * auto [matched, params] = match_path("/users/:id", "/users/123");
 * // matched: true, params: {{"id", "123"}}
 *
 * // Check for malicious content
 * if (body_has_malicious_content(request_body)) {
 *     // Handle security threat
 * }
 * @endcode
 *
 * @note These utilities are used internally by request, route, and router classes
 * @note All functions are thread-safe and stateless
 *
 * @author cppress team
 * @version 1.0
 */

#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace cppress::web {

/**
 * @brief Extract query parameters from a URI.
 * @param uri Full request URI
 * @return Vector of name-value pairs representing query parameters
 *
 * This function parses the query string portion of the URI and extracts
 * all query parameters as name-value pairs. It handles URL decoding
 * and proper parameter separation.
 */
std::map<std::string, std::string> get_query_parameters(const std::string& uri);

/**
 * @brief Check whether a URI points to a static resource by extension.
 * @param uri Request URI
 * @return true if URI extension is in the static_extensions list
 */
bool is_uri_static(const std::string& uri);

/**
 * @brief Extract parameter names from a route expression.
 * @param uri Route expression or URI containing parameter placeholders (e.g., "/users/:id")
 * @return Vector of pairs {param_name, value} where value is empty when only names are extracted
 *
 * Note: This utility only extracts parameter names when the input contains
 * placeholders like ":id". Resolving parameter values from an actual request
 * path requires matching against the route expression and is typically done
 * by the router during request handling.
 */
std::map<std::string, std::string> get_path_params(const std::string& uri);

/**
 * @brief Extract the path component (without query) from a URI.
 * @param uri Full request URI
 * @return Path portion of the URI (everything before '?')
 */
std::string get_path(const std::string& uri);

/**
 * @brief Match a route expression against a request path.
 * @param expression Route expression (may include ":param" and "*" wildcard)
 * @param rhs Actual request path to test
 * @return true if the expression matches the path, false otherwise
 *
 * The rule set implemented by match_path supports:
 * - Exact segment match
 * - Named parameters using leading ':' (matches a single segment)
 */
std::pair<bool, std::map<std::string, std::string>> match_path(const std::string& expression,
                                                               const std::string& rhs);

/**
 * @brief Check if the request body contains malicious content.
 *
 * @param body The request body as a string
 * @return true if malicious content is detected
 * @return false if the body is clean
 */

bool body_has_malicious_content(const std::string& body, bool XSS = true, bool SQL = true,
                                bool CMD = true);
}  // namespace cppress::web
