/**
 * @file includes.hpp
 * @brief Main include file for the cppress web framework
 *
 * @section intro_sec Introduction
 *
 * This is the primary include file for the cppress web framework, a modern C++
 * web framework built on top of cppress HTTP and socket libraries. It provides
 * a high-level, Express.js-inspired API for building web applications and APIs.
 *
 * @section overview_sec Overview
 *
 * The cppress web framework simplifies web development by providing:
 * @li Easy-to-use routing with path parameters
 * @li Middleware support for cross-cutting concerns
 * @li Built-in static file serving
 * @li Request/response abstraction over raw HTTP
 * @li Multi-threaded request handling with worker pools
 * @li Exception handling with HTTP status code integration
 * @li Template-based extensibility for custom request/response types
 *
 * @section architecture_sec Architecture
 *
 * The framework is organized in layers:
 *
 * @subsection core_components Core Components
 * @li server: Main web server class with routing and worker pool
 * @li router: Route management and middleware chains
 * @li route: Individual route definitions with handlers
 * @li request: High-level request wrapper with convenience methods
 * @li response: High-level response wrapper with content-type helpers
 * @li exception: Web-specific exceptions with HTTP status codes
 *
 * @subsection dependencies_sec Dependencies
 * @li cppress::http - HTTP protocol handling (required)
 * @li cppress::sockets - TCP socket communication (required)
 * @li cppress::shared - Thread pool and utilities (required)
 * @li Standard C++ libraries (C++17 or higher)
 *
 * @section capabilities_sec Capabilities
 *
 * @subsection restful_api RESTful API Development
 * @li GET, POST, PUT, DELETE route handlers
 * @li JSON request/response handling
 * @li Path parameters (/users/:id) and query strings (?page=1)
 * @li Custom route patterns and matching
 *
 * @subsection static_files Static File Serving
 * @li Serve HTML, CSS, JS, images, and other static assets
 * @li Multiple static directories
 * @li Automatic MIME type detection
 * @li Path sanitization for security
 *
 * @subsection middleware_system Middleware System
 * @li Pre-route request processing
 * @li Authentication and authorization
 * @li Logging and metrics
 * @li CORS handling
 * @li Request validation
 *
 * @subsection advanced_features Advanced Features
 * @li Multi-threaded request handling
 * @li Configurable worker thread pools
 * @li HTTP keep-alive support
 * @li Cookie management
 * @li Custom error handlers
 * @li Header manipulation
 * @li Request/response lifecycle hooks
 *
 * @subsection security_features Security Features
 * @li Path sanitization to prevent directory traversal
 * @li Malicious content detection (XSS, SQL injection, command injection)
 * @li Secure cookie attributes support
 * @li Exception-safe response handling
 *
 * @section limitations_sec Limitations
 *
 * The framework does NOT support:
 * @li HTTPS/TLS (must be handled by reverse proxy or external layer)
 * @li HTTP/2 or HTTP/3 protocols (HTTP/1.1 only)
 * @li WebSocket connections (different protocol)
 * @li Automatic database integration (bring your own ORM/driver)
 * @li Built-in template engines (We use html lib to create the html documents inside c++ code)
 * @li Automatic request body parsing (JSON parsing is manual, you must include middlewares and use
 the json library yourself)
 * @li Session management (implement custom or use libraries)
 * @li Rate limiting (implement as middleware)
 * @li Compression (gzip, brotli - implement as middleware if needed)
 * @li Multipart form data parsing (implement custom parser)
 * @li File upload handling (implement custom handler)
 * @li Load balancing (use reverse proxy like nginx)
 *
 * @section requirements_sec System Requirements
 *
 * Required Libraries:
 * @li cppress::http (HTTP protocol layer)
 * @li cppress::sockets (TCP socket layer)
 * @li cppress::shared (thread pool, logger, utilities)
 *
 * System Requirements:
 * @li C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
 * @li POSIX-compliant OS for socket operations (Linux, macOS, Unix)
 * @li Multi-threading support (pthread or native threads)
 *
 * @section example_sec Quick Start Example
 *
 * @code{.cpp}
 * #include "web/includes.hpp"
 *
 * int main() {
 *     // Create server on port 8080
 *     auto server = std::make_shared<cppress::web::server<>>(8080);
 *
 *     // Add middleware
 *     server->use([](auto req, auto res) {
 *         std::cout << req->get_method() << " " << req->get_path() << std::endl;
 *         return cppress::web::exit_code::CONTINUE;
 *     });
 *
 *     // Add routes
 *     server->get("/api/hello", {
 *         [](auto req, auto res) {
 *             res->send_json("{\"message\": \"Hello World\"}");
 *             return cppress::web::exit_code::EXIT;
 *         }
 *     });
 *
 *     server->get("/users/:id", {
 *         [](auto req, auto res) {
 *             auto params = req->get_path_params();
 *             res->send_json("{\"userId\": \"" + params["id"] + "\"}");
 *             return cppress::web::exit_code::EXIT;
 *         }
 *     });
 *
 *     // Serve static files
 *     server->use_static("./public");
 *
 *     // Start server
 *     server->listen(
 *         []() { std::cout << "Server running on port 8080" << std::endl; }
 *     );
 *
 *     return 0;
 * }
 * @endcode
 *
 * @section usage_patterns_sec Usage Patterns
 *
 * @subsection custom_routers Creating Custom Routers
 * @code{.cpp}
 * auto apiRouter = std::make_shared<cppress::web::router<>>();
 * apiRouter->get("/products", {  });
 * apiRouter->post("/products", {  });
 * server->use_router(apiRouter);
 * @endcode
 *
 * @subsection error_handling Error Handling
 * @code{.cpp}
 * server->use_error([](auto req, auto res, const cppress::web::exception& e) {
 *     res->set_status(e.get_status_code(), e.get_status_message());
 *     res->send_json("{\"error\": \"" + std::string(e.what()) + "\"}");
 *   });
 * @endcode
 *  server->use_default([](auto req, auto res) {
 *      res->set_status(404, "Not Found");
 *      res->send_html("<h1>Page Not Found</h1>");
 *      return cppress::web::exit_code::EXIT;
 *
 *  });
 * @endcode
 *
 * @section thread_safety_sec Thread Safety
 *
 * @li Request handlers execute in worker threads (thread pool)
 * @li Request/response objects are NOT shared between handlers
 * @li Shared state must be protected with mutexes/locks
 * @li Logger and utilities are thread-safe
 *
 * @section performance_sec Performance Considerations
 *
 * @li Worker pool size defaults to hardware_concurrency()
 * @li Each request is queued to worker pool (non-blocking)
 * @li Static file serving reads from disk (consider caching)
 * @li Keep-alive connections reduce overhead for multiple requests
 * @li Route matching is O(n)
 where n is number of routes** @author cppress team* @version 1.0

*/

#pragma once

#include "includes/exceptions.hpp"
#include "includes/request.hpp"
#include "includes/response.hpp"
#include "includes/route.hpp"
#include "includes/router.hpp"
#include "includes/server.hpp"
#include "includes/types.hpp"
#include "includes/utilities.hpp"
