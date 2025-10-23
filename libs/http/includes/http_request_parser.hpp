/**
 * @file http_request_parser.hpp
 * @brief Internal HTTP protocol parser and message state manager
 *
 * This file contains the core HTTP/1.1 parsing logic used internally by http_server.
 * It handles incremental parsing of HTTP requests that may arrive across multiple
 * TCP segments, maintaining state for each active connection.
 *
 * The parser supports Content-Length based body parsing and automatically manages
 * cleanup of idle connections. All parsing is thread-safe through internal mutex.
 *
 * @note This is an internal implementation detail. Most users should interact with
 *       http_server, http_request, and http_response instead.
 *
 * Key responsibilities:
 * - Parse HTTP request line (method, URI, version)
 * - Parse and validate HTTP headers
 * - Handle partial requests spanning multiple reads
 * - Enforce size limits (MAX_HEADER_SIZE, MAX_BODY_SIZE)
 * - Clean up idle connection state
 * - Validate Content-Length against body size
 *
 * @example Internal usage by http_server
 * @code
 * http_request_parser parser;
 * auto result = parser.parse(connection, data_buffer);
 * if (result.is_complete) {
 *     // Create http_request and http_response objects
 *     // Invoke user callback
 * }
 * @endcode
 */

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>

#include "http_consts.hpp"
#include "http_parse_result.hpp"
#include "http_parse_state.hpp"
#include "sockets/includes.hpp"

namespace cppress::http {

/**
 * @class http_request_parser
 * @brief Thread-safe HTTP message parser with stateful request handling
 *
 * Manages parsing of HTTP requests that may arrive in fragments across multiple
 * TCP read operations. Maintains per-connection state using connection identifiers
 * as keys, allowing concurrent handling of multiple connections.
 *
 * The parser enforces configured size limits and automatically cleans up state
 * for idle connections based on MAX_IDLE_TIME_SECONDS.
 */
class http_request_parser {
    /// Map of connection IDs to incomplete request data
    std::map<std::string, http_parse_state> pending_requests_;

    /// Mutex for thread-safe access to pending_requests_
    std::mutex parser_mutex_;

public:
    /**
     * @brief Main entry point for parsing incoming HTTP data
     * @param conn Client connection that sent the data
     * @param data Raw data buffer received from socket
     * @return http_parse_result with completion status and parsed fields
     *
     * Determines if this is a new request or continuation of existing request,
     * then delegates to appropriate handler (begin_parsing or continue_parsing).
     */
    http_parse_result parse(std::shared_ptr<cppress::sockets::connection> conn,
                            const cppress::sockets::data_buffer& data);

    /**
     * @brief Continue parsing an incomplete request
     * @param state Existing parsing state for this connection
     * @param data Additional data received from socket
     * @return http_parse_result indicating if request is now complete
     *
     * Appends new data to existing body buffer and checks if expected
     * Content-Length has been reached.
     */
    http_parse_result continue_parsing(http_parse_state& state,
                                       const cppress::sockets::data_buffer& data);

    /**
     * @brief Start parsing a new HTTP request
     * @param connection_id Unique identifier for this connection
     * @param data Raw HTTP request data
     * @param socket_fd File descriptor of the socket
     * @return http_parse_result with parsed request components
     *
     * Parses request line and headers, then determines parsing strategy
     * based on Content-Length or chunked encoding (if supported).
     * If body is incomplete, stores state in pending_requests_.
     */
    http_parse_result begin_parsing(const std::string& connection_id,
                                    const cppress::sockets::data_buffer& data, int socket_fd);

    /**
     * @brief Remove idle connections that exceed timeout
     * @param max_idle_time Maximum time connection can be idle
     * @param close_connection Callback to close timed-out connections
     *
     * Iterates through pending_requests_ and closes any connections
     * that have been idle longer than max_idle_time. Should be called
     * periodically by the server (e.g., during idle loop).
     */
    void cleanup_idle_connections(std::chrono::seconds max_idle_time,
                                  std::function<void(int)> close_connection);

private:
    /**
     * @brief Parse the HTTP request line (first line of request)
     * @param request_stream Input stream containing HTTP request
     * @param method Output: HTTP method (GET, POST, etc.)
     * @param uri Output: Request URI
     * @param version Output: HTTP version
     * @return Pair of (success, error_message)
     *
     * Validates format: "METHOD URI VERSION\r\n"
     * Returns false if format is invalid.
     */
    std::pair<bool, std::string> parse_request_line(std::istringstream& request_stream,
                                                    std::string& method, std::string& uri,
                                                    std::string& version);

    /**
     * @brief Parse HTTP headers from request stream
     * @param request_stream Input stream positioned after request line
     * @param uri Request URI (unused, kept for future validation)
     * @param version HTTP version (unused, kept for future validation)
     * @return Pair of (success, headers_multimap)
     *
     * Parses headers until blank line (CRLF CRLF) is encountered.
     * Header names are normalized to uppercase for case-insensitive access.
     * Validates total header size against MAX_HEADER_SIZE.
     */
    std::pair<bool, std::multimap<std::string, std::string>> parse_headers(
        std::istringstream& request_stream, const std::string& uri, const std::string& version);

    /**
     * @brief Check if Transfer-Encoding contains "chunked"
     * @param range Iterator pair for Transfer-Encoding header values
     * @return true if chunked encoding is specified
     *
     * Helper function to detect chunked transfer encoding in headers.
     */
    bool has_chunked_encoding(
        const std::pair<std::multimap<std::string, std::string>::iterator,
                        std::multimap<std::string, std::string>::iterator>& range);

    /**
     * @brief Handle request with Content-Length body
     * @param connection_id Connection identifier
     * @param request_stream Stream positioned at start of body
     * @param method HTTP method
     * @param uri Request URI
     * @param version HTTP version
     * @param headers Parsed request headers
     * @param content_length Expected body size in bytes
     * @param socket_fd Socket file descriptor
     * @return http_parse_result with completion status
     *
     * Reads available body data from stream. If complete body is available,
     * returns is_complete=true. Otherwise stores state and returns is_complete=false.
     * Validates content_length against MAX_BODY_SIZE before buffering.
     */
    http_parse_result parse_content_length_body(
        const std::string& connection_id, std::istringstream& request_stream,
        const std::string& method, const std::string& uri, const std::string& version,
        const std::multimap<std::string, std::string>& headers, size_t content_length,
        int socket_fd);

    /**
     * @brief Continue accumulating body for Content-Length request
     * @param state Existing request state
     * @param data New data chunk received
     * @return http_parse_result indicating if body is now complete
     *
     * Appends data to existing body and checks if expected
     * Content-Length has been reached.
     */
    http_parse_result accumulate_body_data(http_parse_state& state,
                                           const cppress::sockets::data_buffer& data);
};

}  // namespace cppress::http
