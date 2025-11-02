/**
 * @file response.hpp
 * @brief HTTP response builder with safe resource management
 *
 * This file defines the response class, which provides a fluent interface
 * for constructing and sending HTTP responses back to clients. The class manages
 * the response lifecycle including status codes, headers, body, and trailers.
 *
 * The class implements move-only semantics to ensure unique ownership of the
 * socket connection and prevent accidental duplication of response objects.
 *
 * Key features:
 * - Fluent API for building responses (method chaining)
 * - Default 200 OK status with HTTP/1.1
 * - Support for standard and custom headers
 * - Trailer support (headers sent after body)
 * - Automatic Content-Length calculation
 * - Header name normalization (case-insensitive)
 * - Validation before sending
 * - Explicit connection closure via end()
 *
 * The response object is created internally by server and passed to
 * user callbacks alongside request. Users should never construct response
 * directly.
 *
 * @example Basic Usage
 * @code
 * void handle_request(request& req, response& res) {
 *     res.set_status(200, "OK");
 *     res.add_header("Content-Type", "application/json");
 *     res.set_body("{\"message\":\"Hello, World!\"}");
 *     res.send();  // Send response to client
 *     res.end();   // Close connection
 * }
 * @endcode
 *
 * @example Error Response
 * @code
 * res.set_status(404, "Not Found");
 * res.add_header("Content-Type", "text/plain");
 * res.set_body("The requested resource was not found");
 * res.send();
 * res.end();
 * @endcode
 * @note All Headers are sent in UPPERCASE format
 * @note Always call send() before end() to transmit the response
 * @note After calling end(), the response object should not be used further
 *
 * @warning Do not store references or pointers to response objects beyond
 *          the callback scope, as they use move semantics.
 *
 * @see request, server
 */

#pragma once

#include <functional>
#include <map>
#include <string>

namespace cppress::core {
/**
 * @brief Represents an HTTP response with move-only semantics.
 *
 * This class encapsulates an HTTP response including status code, headers,
 * body, and trailers. It provides methods for building and sending HTTP
 * responses while maintaining ownership of the associated client socket.
 *
 * The class implements move-only semantics to ensure unique ownership
 * of the underlying socket resource and prevent accidental duplication
 * of response objects.
 *
 * Features:
 * - Default HTTP/1.1 with 200 OK status
 * - Support for headers and trailers
 * - Automatic validation before sending
 * - Safe resource management
 */
class response {
private:
    /// HTTP version (defaults to "HTTP/1.1")
    std::string version = "HTTP/1.1";

    /// HTTP status code (defaults to 200)
    int status_code = 200;

    /// HTTP status message (defaults to "OK")
    std::string status_message = "OK";

    /// HTTP headers (multimap allows multiple values per header name)
    std::multimap<std::string, std::string> headers;

    /// HTTP trailers (sent after body in chunked encoding)
    std::multimap<std::string, std::string> trailers;

    /// Response body content
    std::string body;

    /// Function to close the connection when needed (closes the current client only, it shall know
    /// what to close)
    std::function<void()> close_connection;

    /// Function to send a message to the client
    std::function<void(const std::string&)> send_message;

    /**
     * @brief Get standard status text for a status code.
     * @param code HTTP status code
     * @return Standard status text (e.g., "OK" for 200)
     */
    static std::string get_status_text(int code);

    /**
     * @brief Validate the response before sending.
     * @return true if response is valid, false otherwise
     *
     * Performs validation checks on the response to ensure it conforms
     * to HTTP standards before transmission.
     */
    bool validate() const;

public:
    /**
     * @brief Private constructor for internal use by server.
     * @param version HTTP version
     * @param headers Initial headers
     * @param close_connection Function to close the associated connection
     */
    response(std::function<void()> close_connection,
             std::function<void(const std::string&)> send_message);

    // Copy operations - DELETED for resource safety
    /**
     * @brief Copy constructor - DELETED.
     *
     * Copy construction is disabled to prevent duplication of socket
     * resources and ensure unique ownership of the client connection.
     */
    response(const response&) = delete;

    /**
     * @brief Copy assignment - DELETED.
     *
     * Copy assignment is disabled to maintain unique ownership semantics.
     */
    response& operator=(const response&) = delete;

    /**
     * @brief Move assignment
     *
     * Transfers ownership of the response data and socket connection.
     * The source object becomes invalid after the move.
     */
    response& operator=(response&&);

    // Move construction - ENABLED for ownership transfer
    /**
     * @brief Move constructor.
     * @param other Response object to move from
     *
     * Transfers ownership of the response data and socket connection.
     * The source object becomes invalid after the move.
     */
    response(response&& other);

    /**
     * @brief Convert response to HTTP string format.
     */
    std::string to_string() const;

    /**
     * @brief Set the response body content.
     * @param body Response body content
     */
    void set_body(const std::string& body);

    /**
     * @brief Set the HTTP status code and message.
     */
    void set_status(int status_code, const std::string& status_message);

    /**
     * @brief Set the HTTP version.
     * @param version HTTP version string (e.g., "HTTP/1.1")
     */
    void set_version(const std::string& version);

    /**
     * @brief Add a trailer header.
     * @param name Trailer name
     */
    void add_trailer(const std::string& name, const std::string& value);

    /**
     * @brief Add a response header.
     * @param name Header name
     * @param value Header value

     */
    void add_header(const std::string& name, const std::string& value);

    /**
     * @brief Get the response body.
     * @return Response body content
     */
    std::string get_body() const;

    /**
     * @brief Get the HTTP version.
     */
    std::string get_version() const;

    /**
     * @brief Get the status message.
     */
    std::string get_status_message() const;

    /**
     * @brief Get the status code.
     */
    int get_status_code() const;

    /**
     * @brief Get all values for a specific header.
     */
    std::vector<std::string> get_header(const std::string& name) const;

    /**
     * @brief Get all values for a specific trailer.
     */
    std::vector<std::string> get_trailer(const std::string& name) const;

    /**
     * @brief Check if a header exists.
     * @param name Header name (case-insensitive)
     * @return true if header exists, false otherwise
     */
    bool has_header(const std::string& name) const;

    /**
     * @brief Remove all values for a specific header.
     * @param name Header name (case-insensitive)
     */
    void remove_header(const std::string& name);

    /**
     * @brief Set a header value (replaces existing values).
     * @param name Header name
     * @param value Header value
     *
     * Unlike add_header(), this method removes any existing values
     * for the header before adding the new value.
     */
    void set_header(const std::string& name, const std::string& value);

    /**
     * @brief Send a JSON response.
     * @param json_data JSON string to send
     * @param status_code HTTP status code (default: 200)
     *
     * Sets Content-Type to application/json, sets the body,
     * and sends the response. Does NOT close the connection.
     *
     * @code
     * res.json("{\"message\": \"Success\"}");
     * res.end();
     * @endcode
     */
    void json(const std::string& json_data, int status_code = 200);

    /**
     * @brief Send an HTML response.
     * @param html_content HTML string to send
     * @param status_code HTTP status code (default: 200)
     *
     * Sets Content-Type to text/html, sets the body,
     * and sends the response. Does NOT close the connection.
     */
    void html(const std::string& html_content, int status_code = 200);

    /**
     * @brief Send a plain text response.
     * @param text_content Plain text to send
     * @param status_code HTTP status code (default: 200)
     *
     * Sets Content-Type to text/plain, sets the body,
     * and sends the response. Does NOT close the connection.
     */
    void text(const std::string& text_content, int status_code = 200);

    /**
     * @brief Send a redirect response.
     * @param location URL to redirect to
     * @param status_code HTTP status code (default: 302 Found)
     *
     * Sets the Location header and sends a redirect response.
     * Common status codes: 301 (Moved Permanently), 302 (Found),
     * 303 (See Other), 307 (Temporary Redirect), 308 (Permanent Redirect)
     */
    void redirect(const std::string& location, int status_code = 302);

    /**
     * @brief Set status code with a fluent interface.
     * @param code HTTP status code
     *
     * @code
     * res.status(404);
     * res.send();
     * res.end(); // to close the connection
     * @endcode
     */
    void status(int code);

    /**
     * @brief ends the response, closes the connection with the client
     *
     * @note After calling end(), the response object should not be
     *       used for further operations
     */
    void end();

    /**
     * @brief Send the HTTP response.
     *
     * This function sends the constructed HTTP response back to the client
     * over the established socket connection.
     */
    void send();

    /**
     * @brief Clear all values for a specific header.
     * @param name Header name
     * @deprecated Use remove_header() instead for consistency
     */
    void clear_header_values(const std::string& name) { remove_header(name); }

    /**
     * @brief Send the HTTP trailers.
     *
     * This function sends any trailers that have been added to the response.
     * Trailers are sent after the response body and headers.
     */

    void send_trailers();

    /// Default destructor
    ~response() = default;
};
}  // namespace cppress::core