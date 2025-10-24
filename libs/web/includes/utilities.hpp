#pragma once

#include <string>
#include <unordered_map>
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
std::vector<std::pair<std::string, std::string>> get_query_parameters(const std::string& uri);

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
std::vector<std::pair<std::string, std::string>> get_path_params(const std::string& uri);

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
std::pair<bool, std::vector<std::pair<std::string, std::string>>> match_path(
    const std::string& expression, const std::string& rhs);

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
