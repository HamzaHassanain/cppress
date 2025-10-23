/**
 * @file http_parse_state.hpp
 * @brief Internal state structure for incomplete HTTP requests
 *
 * This file defines POD (Plain Old Data) structures used internally by
 * http_request_parser to track parsing state for HTTP requests that
 * arrive across multiple TCP segments.
 *
 * When a request's body is not completely received in the first read
 * operation (common for large POST/PUT requests), the parser stores
 * intermediate parsing state in http_parse_state until the
 * complete request is assembled.
 *
 * @note This is an internal implementation detail not exposed to users
 */

#pragma once

#include <chrono>
#include <map>
#include <string>

namespace cppress::http {

/**
 * @enum parse_strategy
 * @brief Strategy for handling request body based on HTTP headers
 */
enum class parse_strategy {
    CONTENT_LENGTH,    ///< Body size specified via Content-Length header
    CHUNKED_ENCODING,  ///< Chunked transfer encoding (not yet implemented)
    NONE               ///< No recognized body encoding detected
};

/**
 * @struct http_parse_state
 * @brief Temporary storage for incomplete HTTP request parsing state
 *
 * Stored in http_request_parser::pending_requests_ map, keyed by
 * connection identifier. Accumulates headers and body data across multiple
 * read operations until the complete request is received.
 *
 * The last_activity timestamp enables automatic cleanup of stale
 * connections that have been idle beyond MAX_IDLE_TIME_SECONDS.
 */
struct http_parse_state {
    /// Unique identifier for client connection (remote address string)
    std::string connection_id;

    /// File descriptor of the socket
    int socket_fd;

    /// Parsing strategy based on request headers
    parse_strategy strategy;

    /// Expected total body size (when strategy == CONTENT_LENGTH)
    std::size_t expected_body_length;

    /// HTTP method (GET, POST, PUT, DELETE, etc.)
    std::string method;

    /// Request URI/path
    std::string uri;

    /// HTTP version (typically "HTTP/1.1")
    std::string http_version;

    /// Request headers (multimap allows duplicate header names)
    std::multimap<std::string, std::string> headers;

    /// Accumulated request body (filled incrementally)
    std::string accumulated_body;

    /// Timestamp of last data received for this connection
    std::chrono::steady_clock::time_point last_activity;

    /// Default constructor
    http_parse_state() = default;

    /**
     * @brief Construct with connection ID and parsing strategy
     * @param conn_id Client identifier
     * @param strat Body parsing strategy
     */
    http_parse_state(const std::string& conn_id, parse_strategy strat)
        : connection_id(conn_id), strategy(strat) {}

    /**
     * @brief Comparison operator for map storage
     * @param other Another http_parse_state instance
     * @return true if this connection_id < other.connection_id
     */
    bool operator<(const http_parse_state& other) const {
        return connection_id < other.connection_id;
    }
};

}  // namespace cppress::http
