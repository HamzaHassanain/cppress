/**
 * @file http_request.hpp
 * @brief HTTP request representation with safe resource management
 *
 * This file defines the http_request class, which encapsulates an incoming
 * HTTP request including all its components (method, URI, version, headers, body).
 *
 * The class implements move-only semantics to ensure unique ownership of the
 * underlying socket connection. This prevents resource leaks and ensures that
 * each request has exactly one owner throughout its lifetime.
 *
 * Key features:
 * - Immutable request data (read-only access via getters)
 * - Automatic connection cleanup on destruction
 * - Case-insensitive header access (headers normalized to uppercase internally)
 * - Support for multiple values per header name
 * - Safe destroy() method with confirmation parameter
 *
 * The http_request object is created internally by http_server and passed to
 * user callbacks. Users should never construct http_request directly.
 *
 * @example
 * @code
 * void handle_request(http_request& req, http_response& res) {
 *     std::string method = req.get_method();
 *     std::string uri = req.get_uri();
 *     auto content_type = req.get_header("Content-Type");
 *     std::string body = req.get_body();
 *
 *     // Process request and build response...
 * }
 * @endcode
 *
 * @see http_response, http_server
 */

#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "http_consts.hpp"

namespace cppress::http {
/**
 * @brief Represents an HTTP request with move-only semantics.
 *
 * This class encapsulates an HTTP request including method, URI, version,
 * headers, and body. It provides a safe interface for accessing request
 * data while maintaining ownership of the associated client socket.
 *
 * The class implements move-only semantics to ensure unique ownership
 * of the underlying socket resource and prevent accidental duplication
 * of request objects.
 */
class http_request {
private:
    /// HTTP method (GET, POST, PUT, DELETE, etc.)
    std::string method;

    /// Request URI/path
    std::string uri;

    /// HTTP version (e.g., "HTTP/1.1")
    std::string version;

    /// HTTP headers (multimap allows multiple values per header name)
    std::multimap<std::string, std::string> headers;

    /// Request body content
    std::string body;

public:
    /// Function to close the connection when needed (closes the current client only, it shall know
    /// what to close)
    std::function<void()> close_connection;

    /**
     * @brief Private constructor for internal use by http_server.
     * @param method HTTP method
     * @param uri Request URI
     * @param version HTTP version
     * @param headers Request headers
     * @param body Request body
     * @param close_connection Function to close the associated connection
     */
    http_request(const std::string& method, const std::string& uri, const std::string& version,
                 const std::multimap<std::string, std::string>& headers, const std::string& body,
                 std::function<void()> close_connection);
    // Copy operations - DELETED for resource safety
    /**
     * @brief Copy constructor - DELETED.
     *
     * Copy construction is disabled to prevent duplication of socket
     * resources and ensure unique ownership of the client connection.
     */
    http_request(const http_request&) = delete;

    /**
     * @brief Copy assignment - DELETED.
     *
     * Copy assignment is disabled to maintain unique ownership semantics.
     */
    http_request& operator=(const http_request&) = delete;

    /**
     * @brief Move assignment - DELETED.
     *
     * Move assignment is disabled to prevent reassignment after construction.
     * Use move construction instead for transferring ownership.
     */
    http_request& operator=(http_request&&) = delete;

    // Move construction - ENABLED for ownership transfer
    /**
     * @brief Move constructor.
     * @param other Request object to move from
     *
     * Transfers ownership of the request data and socket connection.
     * The source object becomes invalid after the move.
     */
    http_request(http_request&& other);

    /**
     * @brief Get the HTTP method.
     * @return HTTP method string (GET, POST, PUT, DELETE, etc.)
     */
    std::string get_method() const;

    /**
     * @brief Get the request URI.
     * @return Request URI/path
     */
    std::string get_uri() const;

    /**
     * @brief Get the HTTP version.
     * @return HTTP version string (e.g., "HTTP/1.1")
     */
    std::string get_version() const;

    /**
     * @brief Get all values for a specific header.
     * @param name Header name (case-insensitive)
     * @return Vector of header values
     */
    std::vector<std::string> get_header(const std::string& name) const;

    /**
     * @brief Get all headers as name-value pairs.
     * @return Vector of header name-value pairs
     */
    std::vector<std::pair<std::string, std::string>> get_headers() const;

    /**
     * @brief Get the request body.
     * @return Request body content
     */
    std::string get_body() const;

    /**
     * @brief Check if a header exists.
     * @param name Header name (case-insensitive)
     * @return true if header exists, false otherwise
     */
    bool has_header(const std::string& name) const;

    /**
     * @brief Get a query parameter from the URI.
     * @param name Parameter name
     * @return Parameter value, or empty string if not found
     *
     * Parses the query string from the URI (e.g., /path?name=value)
     * and returns the value for the specified parameter name.
     *
     * @code
     * // For URI: /search?q=test&page=2
     * std::string query = req.get_query_param("q");  // Returns "test"
     * std::string page = req.get_query_param("page");  // Returns "2"
     * @endcode
     */
    std::string get_query_param(const std::string& name) const;

    /**
     * @brief Get all query parameters as name-value pairs.
     * @return Map of parameter names to values
     */
    std::map<std::string, std::string> get_query_params() const;

    /**
     * @brief Get the path portion of the URI (without query string).
     * @return Path without query parameters
     *
     * @code
     * // For URI: /search?q=test
     * std::string path = req.get_path();  // Returns "/search"
     * @endcode
     */
    std::string get_path() const;

    /// Default destructor
    ~http_request() = default;
};
}  // namespace cppress::http