/**
 * @file includes.hpp
 * @brief Main header file for the cppress HTTP module
 *
 * @section overview Overview
 * This module provides a complete HTTP/1.1 server implementation built on top of
 * the cppress sockets library. It handles low-level HTTP protocol parsing, request/response
 * management, and provides a clean callback-driven API for building web applications.
 *
 * @section architecture Architecture
 * The HTTP module is designed with a layered architecture:
 *
 * 1. **Protocol Layer** (http_message_handler):
 *    - Handles HTTP/1.1 parsing according to RFC 2616/7230
 *    - Manages partial request handling for streaming data
 *    - Supports Content-Length based body parsing
 *    - Validates request format and headers
 *    - Thread-safe internal state management
 *
 * 2. **Request/Response Layer** (http_request, http_response):
 *    - Provides safe, move-only wrappers for HTTP messages
 *    - Enforces unique ownership of socket connections
 *    - Prevents resource duplication and leaks
 *    - Headers automatically normalized (case-insensitive)
 *    - Immutable request objects for safety
 *
 * 3. **Server Layer** (http_server):
 *    - Built on epoll-based TCP server (Linux) / select (cross-platform)
 *    - Callback-driven or inheritance-based request handling
 *    - Automatic connection lifecycle management
 *    - Configurable timeouts and size limits
 *    - Support for multiple concurrent connections
 *
 * @section features Features
 *
 * **What This Module Provides:**
 * - ✅ HTTP/1.1 request parsing (GET, POST, PUT, DELETE, etc.)
 * - ✅ Content-Length based body handling
 * - ✅ Header parsing with case-insensitive access
 * - ✅ Configurable maximum header/body sizes
 * - ✅ Connection timeout management
 * - ✅ Move-only request/response semantics for safety
 * - ✅ Thread-safe request handling (when using thread pool)
 * - ✅ Callback-driven architecture for clean separation of concerns
 * - ✅ HTTP status code and header constants
 * - ✅ Automatic "Connection: close" handling
 * - ✅ Support for custom headers and trailers
 *
 * **What This Module Does NOT Provide:**
 * - ❌ HTTP/2 or HTTP/3 protocol support
 * - ❌ Chunked transfer encoding (planned for future)
 * - ❌ Automatic compression (gzip, brotli, etc.)
 * - ❌ Keep-alive / persistent connections (planned for future)
 * - ❌ Range requests / partial content
 * - ❌ Multipart form-data parsing (use external parser)
 * - ❌ SSL/TLS support (add using reverse proxy)
 *
 * @section usage Basic Usage
 *
 * @subsection callback_example Callback-Based Server
 * @code
 * #include "http/includes.hpp"
 *
 * void handle_request(cppress::http::http_request& req,
 *                     cppress::http::http_response& res) {
 *     res.set_status(200, "OK");
 *     res.add_header("Content-Type", "text/plain");
 *     res.set_body("Hello, World!");
 *     res.send();
 *     res.end();  // Close connection
 * }
 *
 * int main() {
 *     cppress::http::http_server server(8080, "0.0.0.0");
 *     server.set_request_callback(handle_request);
 *     server.listen();  // Blocks until server stops
 * }
 * @endcode
 *
 * @subsection inheritance_example Inheritance-Based Server
 * @code
 * class MyServer : public cppress::http::http_server {
 * protected:
 *     void on_request_received(http_request& req, http_response& res) override {
 *         if (req.get_uri() == "/api/data") {
 *             res.set_body(get_data());
 *         }
 *         res.send();
 *         res.end();
 *     }
 * public:
 *     MyServer(int port) : http_server(port) {}
 * };
 * @endcode
 *
 * @section configuration Configuration
 *
 * Global settings can be adjusted via the config namespace:
 * @code
 * cppress::http::config::MAX_HEADER_SIZE = 32 * 1024;  // 32 KB
 * cppress::http::config::MAX_BODY_SIZE = 10 * 1024 * 1024;  // 10 MB
 * cppress::http::config::MAX_IDLE_TIME_SECONDS = std::chrono::seconds(30);
 * cppress::http::config::TIMEOUT_MILLISECONDS = 5000;  // 5 seconds
 * @endcode
 *
 * @section threading Thread Safety
 *
 * - The http_message_handler uses internal mutexes for thread safety
 * - Multiple requests can be handled concurrently using a thread pool
 * - Request/response objects are move-only and not thread-safe themselves
 * - Callbacks should avoid blocking operations (offload to thread pool)
 *
 * @section best_practices Best Practices
 *
 * 1. **Always call res.end()**: Close connections explicitly after sending response
 * 2. **Use thread pools**: Offload heavy processing from request callbacks
 * 3. **Validate input**: Check request method, URI, headers before processing
 * 4. **Set size limits**: Configure MAX_BODY_SIZE based on your needs
 * 5. **Handle errors**: Set error_callback to catch and log exceptions
 * 6. **Don't store references**: Request/response objects are move-only
 * 7. **Check Content-Length**: Validate expected body size for POST/PUT
 *
 * @section internal_parsing Internal Parsing Details
 *
 * The http_request_parser maintains state for incomplete requests:
 * - Requests arriving in multiple TCP segments are buffered internally
 * - State stored per-connection using connection identifier as key
 * - Idle connections automatically cleaned up after MAX_IDLE_TIME_SECONDS
 * - Content-Length is validated against MAX_BODY_SIZE before buffering
 * - Headers normalized to uppercase for case-insensitive matching
 * - Malformed requests result in connection closure (no 400 response sent)
 *
 * @section limitations Known Limitations
 *
 * - Current implementation uses "Connection: close" for all responses
 * - No pipeline support (one request per connection)
 * - Chunked encoding not yet implemented
 * - No built-in support for 100-continue
 * - Limited to HTTP/1.1 protocol
 *
 * @section dependencies Dependencies
 *
 * This module depends on:
 * - cppress::sockets (TCP server implementation)
 * - cppress::shared (utility functions like to_uppercase)
 * - Standard C++17 library
 *
 * @author Hamza Hassanain
 * @version 1.0.0
 */

#pragma once

#include "includes/http_consts.hpp"
#include "includes/http_request.hpp"
#include "includes/http_response.hpp"
#include "includes/http_server.hpp"