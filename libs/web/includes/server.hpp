/**
 * @file server.hpp
 * @brief High-level web server implementation with routing and middleware support
 *
 * This file defines the server class, the main entry point for creating web applications
 * in the cppress web framework. It combines HTTP protocol handling with routing,
 * middleware, static file serving, and multi-threaded request processing.
 *
 * @section server_features Key Features
 * @li Multi-threaded request processing with configurable worker pool
 * @li Static file serving with automatic MIME type detection
 * @li Router registration for organizing routes
 * @li Middleware support for cross-cutting concerns
 * @li Exception handling with HTTP status code integration
 * @li HTTP keep-alive connection support
 * @li Custom error and 404 handlers
 * @li Header processing callbacks
 *
 * @section server_usage Usage Example
 * @code{.cpp}
 * // Create server on port 8080 with 4 worker threads
 * auto server = std::make_shared<cppress::web::server<>>(8080, "0.0.0.0", 4);
 *
 * // Add global middleware
 * server->use([](auto req, auto res) {
 *     std::cout << "Request: " << req->get_path() << std::endl;
 *     return cppress::web::exit_code::CONTINUE;
 * });
 *
 * // Add routes
 * server->get("/api/users", {
 *     [](auto req, auto res) {
 *         res->send_json("{\"users\": []}");
 *         return cppress::web::exit_code::EXIT;
 *     }
 * });
 *
 * // Serve static files from ./public directory
 * server->use_static("./public");
 *
 * // Custom 404 handler
 * server->use_default([](auto req, auto res) {
 *     res->set_status(404, "Not Found");
 *     res->send_json("{\"error\": \"Not Found\"}");
 *     return cppress::web::exit_code::EXIT;
 * });
 *
 * // Custom error handler
 * server->use_error([](auto req, auto res, const auto& e) {
 *     res->set_status(e.get_status_code(), e.get_status_message());
 *     res->send_json("{\"error\": \"" + std::string(e.what()) + "\"}");
 * });
 *
 * // Start server
 * server->listen(
 *     []() { std::cout << "Server started!" << std::endl; },
 *     [](const auto& e) { std::cerr << "Error: " << e.what() << std::endl; }
 * );
 * @endcode
 *
 * @section server_architecture Architecture
 *
 * The server class inherits from cppress::http::http_server and adds:
 * @li Worker thread pool for concurrent request handling
 * @li Route matching and handler execution
 * @li Static file serving with path sanitization
 * @li Middleware chain execution
 * @li Exception handling and error responses
 *
 * @section server_request_flow Request Processing Flow
 * -# Request received by HTTP server layer
 * -# Request/response converted to web objects
 * -# Request queued to worker thread pool
 * -# Worker executes request_handler():
 *    - Check if URI is static file
 *    - If static, serve file and return
 *    - If dynamic, try each router in order
 *    - If no match, call default handler (404)
 * -# Send response and close/keep-alive connection
 *
 * @section server_templates Template Parameters
 * @tparam T Request type (must derive from cppress::web::request)
 * @tparam G Response type (must derive from cppress::web::response)
 * @tparam R Router type (must derive from cppress::web::router<T, G>)
 *
 * @note The server automatically creates a base router at index 0
 * @note All route shortcut methods (get, post, etc.) use the base router
 * @note Server objects are non-copyable and non-movable
 *
 * @author cppress team
 * @version 1.0
 */

#pragma once

#include <iostream>
#include <thread>

#include "../includes.hpp"
#include "exceptions.hpp"
#include "http/includes.hpp"
#include "shared/includes/thread_pool.hpp"
#include "shared/includes/utils.hpp"

#define HEADER_RECEIVED_PARAMS                                                            \
    std::shared_ptr<cppress::sockets::connection> conn,                                   \
        const std::multimap<std::string, std::string>&headers, const std::string &method, \
        const std::string &uri, const std::string &version, const std::string &body
namespace cppress::web {
/**
 * @class server
 * @brief High-level web server template for handling HTTP requests with routing
 *
 * This class provides a complete web server implementation built on top of the
 * lower-level HTTP server. It manages request routing, static file serving,
 * middleware execution, and worker thread pools for concurrent request handling.
 *
 * @section server_class_features Key Features
 * @li Multi-threaded request processing with worker pool
 * @li Static file serving with MIME type detection
 * @li Router registration for dynamic content
 * @li Middleware support for cross-cutting concerns
 * @li Exception handling with proper HTTP status codes
 * @li HTTP keep-alive connection support
 * @li Customizable error and 404 handlers
 *
 * @tparam T Request type (must derive from request)
 * @tparam G Response type (must derive from response)
 * @tparam R Router type (must derive from router<T, G>)
 */
template <typename T = request, typename G = response, typename R = router<T, G>>
class server : public cppress::http::http_server {
protected:
    /// Server port number
    int port;

    /// Server host/IP address
    std::string host;

    /// Thread pool for handling requests concurrently
    shared::thread_pool worker_pool;

    /// Directories to serve static files from
    std::vector<std::string> static_directories;

    /// Registered routers for handling dynamic requests
    std::vector<std::shared_ptr<R>> routers;

    /**
     * @brief Callback executed when server starts listening
     *
     * Default implementation prints server address and port to stdout.
     * Can be overridden via listen() method.
     */
    listen_callback_t listen_callback = [this]() -> void {
        std::cout << "Server is listening at " << this->host << ":" << this->port << std::endl;
    };

    /**
     * @brief Callback for handling server errors
     *
     * Default implementation logs errors using the shared logger.
     * Can be overridden via listen() method.
     */
    std::function<void(const std::exception&)> error_callback =
        [](const std::exception& e) -> void {
        std::string what = e.what();
        shared::logger::error("[Socket Exception]: " + what);
    };

    /**
     * @brief Callback for header processing
     *
     * Called when HTTP headers are received but before body processing.
     * Allows for early request inspection, logging, or connection termination.
     * Set via use_headers_received() method.
     */
    std::function<void(HEADER_RECEIVED_PARAMS)> headers_callback = nullptr;

    /**
     * @brief Handler for unmatched routes (404 responses)
     *
     * Default implementation returns 404 Not Found with plain text message.
     * Can be customized via use_default() method.
     */
    request_handler_t<T, G> handle_default_route = []([[maybe_unused]] std::shared_ptr<T> req,
                                                      std::shared_ptr<G> res) -> exit_code {
        res->set_status(404, "Not Found");
        res->send_text("404 Not Found");
        return exit_code::EXIT;
    };

    /**
     * @brief Callback for handling unhandled exceptions
     *
     * Called when an exception occurs during request processing.
     * If not set, a default 500 error response is sent.
     * Set via use_error() method.
     */
    unhandled_exception_callback_t<T, G> unhandled_exception_callback = nullptr;

public:
    /**
     * @brief Construct a web server with specified port and host
     *
     * Creates a new web server instance and initializes the worker thread pool.
     * Automatically creates a base router at index 0 for use with shortcut methods
     * (get, post, put, delete_, use).
     *
     * @param port Port number to listen on (1-65535)
     * @param host Host address to bind to (default: "0.0.0.0" for all interfaces)
     * @param worker_threads Number of worker threads in the pool (default: hardware concurrency)
     *
     * @throws std::invalid_argument if port is invalid or worker_threads is 0
     *
     * @note The server validates template parameters at compile-time using static_assert
     * @note Base router is created automatically and accessible via index 0
     */
    explicit server(int port, const std::string& host = "0.0.0.0",
                    std::size_t worker_threads = std::thread::hardware_concurrency())
        : cppress::http::http_server(port, host),
          port(port),
          host(host),
          worker_pool(worker_threads) {
        static_assert(std::is_base_of<request, T>::value, "T must derive from request");
        static_assert(std::is_base_of<response, G>::value, "G must derive from response");
        static_assert(std::is_base_of<router<T, G>, R>::value, "R must derive from router<T, G>");

        auto base_router = std::make_shared<R>();
        this->routers.push_back(base_router);
    }

    /**
     * @brief Deleted copy constructor
     *
     * Server objects cannot be copied to ensure unique ownership of resources
     * (sockets, thread pools, routers).
     */
    server(const server&) = delete;

    /**
     * @brief Deleted copy assignment operator
     *
     * Server objects cannot be copied to ensure unique ownership of resources.
     */
    server& operator=(const server&) = delete;

    /**
     * @brief Deleted move constructor
     *
     * Server objects cannot be moved to prevent resource management issues.
     */
    server(server&&) = delete;

    /**
     * @brief Deleted move assignment operator
     *
     * Server objects cannot be moved to prevent resource management issues.
     */
    server& operator=(server&&) = delete;

    /**
     * @brief Register a router for handling dynamic requests
     *
     * Adds a router to the server's collection. Routers are checked in the order
     * they are registered, so the first matching route will handle the request.
     *
     * @param router Shared pointer to the router to register
     *
     * @note Routers are checked after the base router (index 0)
     * @note Multiple routers can be registered for organizing routes by feature/module
     */
    virtual void use_router(std::shared_ptr<router<T, G>> router) {
        this->routers.push_back(router);
    }

    /**
     * @brief Register a directory for serving static files
     *
     * Files from registered directories will be served automatically when the URI
     * matches a static file extension. Multiple directories can be registered,
     * and they are checked in registration order.
     *
     * @param directory Path to the static files directory (absolute or relative)
     *
     * @note Directory path is used as-is; ensure it ends without trailing slash
     * @note Files are served with automatic MIME type detection
     * @note Path sanitization is applied to prevent directory traversal attacks
     *
     * Example:
     * @code{.cpp}
     * server->use_static("./public");     // Serves ./public/index.html for /index.html
     * server->use_static("/var/www");     // Serves /var/www/style.css for /style.css
     * @endcode
     */
    virtual void use_static(const std::string& directory) {
        static_directories.push_back(directory);
    }

    /**
     * @brief Set custom handler for unmatched routes
     *
     * Replaces the default 404 handler with a custom implementation.
     * This handler is called when no router matches the request.
     *
     * @param handler Function to handle 404 cases
     *
     * Example:
     * @code{.cpp}
     * server->use_default([](auto req, auto res) {
     *     res->set_status(404, "Not Found");
     *     res->send_json("{\"error\": \"Resource not found\"}");
     *     return cppress::web::exit_code::EXIT;
     * });
     * @endcode
     */
    virtual void use_default(const request_handler_t<T, G>& handler) {
        handle_default_route = handler;
    }

    /**
     * @brief Register a callback for when headers are received
     *
     * This callback is invoked after HTTP headers are parsed but before the
     * request body is processed. Useful for logging, early validation, or
     * closing connections based on headers.
     *
     * @param callback Function to call with header information
     *
     * @note Default behavior is no action (callback is nullptr)
     * @note You can use close_connection(conn) to terminate the connection early
     *
     * Example:
     * @code{.cpp}
     * server->use_headers_received([](auto conn, auto headers, auto method,
     *                                 auto uri, auto version, auto body) {
     *     std::cout << method << " " << uri << std::endl;
     * });
     * @endcode
     */
    virtual void use_headers_received(const std::function<void(HEADER_RECEIVED_PARAMS)>& callback) {
        headers_callback = callback;
    }

    /**
     * @brief Register a callback for unhandled exceptions
     *
     * Sets a custom handler for exceptions that occur during request processing.
     * If not set, the server sends a default 500 error response.
     *
     * @param callback Function to handle exceptions
     *
     * @note Default behavior logs the error and sends 500 Internal Server Error
     * @note The callback receives the request, response, and exception objects
     *
     * Example:
     * @code{.cpp}
     * server->use_error([](auto req, auto res, const cppress::web::exception& e) {
     *     res->set_status(e.get_status_code(), e.get_status_message());
     *     res->send_json("{\"error\": \"" + std::string(e.what()) + "\"}");
     * });
     * @endcode
     */
    virtual void use_error(unhandled_exception_callback_t<T, G> callback) {
        unhandled_exception_callback = callback;
    }

    /**
     * @brief Start the server and begin listening for requests
     *
     * Starts the HTTP server and begins accepting connections. This is a blocking
     * call that runs until the server is stopped or an error occurs.
     *
     * @param listen_callback Optional callback invoked when server starts successfully
     * @param error_callback Optional callback invoked when an error occurs
     *
     * @note If callbacks are provided, they replace the default callbacks
     * @note This method blocks until server shutdown
     * @note Worker threads are already started before this call
     *
     * Example:
     * @code{.cpp}
     * server->listen(
     *     []() { std::cout << "Server started!" << std::endl; },
     *     [](const auto& e) { std::cerr << "Error: " << e.what() << std::endl; }
     * );
     * @endcode
     */
    virtual void listen(listen_callback_t listen_callback = nullptr,
                        error_callback_t error_callback = nullptr) {
        if (listen_callback) {
            this->listen_callback = listen_callback;
        }
        if (error_callback) {
            this->error_callback = error_callback;
        }
        cppress::http::http_server::listen();
    }

    /**
     * @brief Stop the server and shutdown worker threads
     *
     * Gracefully shuts down the HTTP server and stops all worker threads.
     * Any pending requests in the worker queue will be completed before shutdown.
     *
     * @note This method should be called to properly clean up resources
     * @note Blocks until all worker threads have finished
     */
    virtual void stop() {
        cppress::http::http_server::shutdown();
        worker_pool.stop_workers();
    }

    /**
     * @brief Register a GET route for the base router
     *
     * Convenience method to add a GET route to the default router (index 0).
     *
     * @param path The path pattern for the route (e.g., "/users/:id")
     * @param handlers Vector of request handlers to execute for this route
     *
     * Example:
     * @code{.cpp}
     * server->get("/api/users", {
     *     [](auto req, auto res) {
     *         res->send_json("{\"users\": []}");
     *         return cppress::web::exit_code::EXIT;
     *     }
     * });
     * @endcode
     */
    void get(const std::string& path, std::vector<request_handler_t<T, G>> handlers) {
        routers[0]->add_route(std::make_shared<route<T, G>>("GET", path, handlers));
    }

    /**
     * @brief Register a POST route for the base router
     *
     * Convenience method to add a POST route to the default router (index 0).
     *
     * @param path The path pattern for the route
     * @param handlers Vector of request handlers to execute for this route
     */
    void post(const std::string& path, std::vector<request_handler_t<T, G>> handlers) {
        routers[0]->add_route(std::make_shared<route<T, G>>("POST", path, handlers));
    }

    /**
     * @brief Register a PUT route for the base router
     *
     * Convenience method to add a PUT route to the default router (index 0).
     *
     * @param path The path pattern for the route
     * @param handlers Vector of request handlers to execute for this route
     */
    void put(const std::string& path, std::vector<request_handler_t<T, G>> handlers) {
        routers[0]->add_route(std::make_shared<route<T, G>>("PUT", path, handlers));
    }

    /**
     * @brief Register a DELETE route for the base router
     *
     * Convenience method to add a DELETE route to the default router (index 0).
     *
     * @param path The path pattern for the route
     * @param handlers Vector of request handlers to execute for this route
     *
     * @note Method named delete_ (with underscore) to avoid C++ keyword conflict
     */
    void delete_(const std::string& path, std::vector<request_handler_t<T, G>> handlers) {
        routers[0]->add_route(std::make_shared<route<T, G>>("DELETE", path, handlers));
    }

    /**
     * @brief Register middleware for the base router
     *
     * Adds middleware to the default router (index 0). Middleware executes before
     * route handlers and can perform cross-cutting concerns like logging,
     * authentication, or request modification.
     *
     * @param middleware The middleware function to register
     *
     * Example:
     * @code{.cpp}
     * server->use([](auto req, auto res) {
     *     std::cout << req->get_method() << " " << req->get_path() << std::endl;
     *     return cppress::web::exit_code::CONTINUE;
     * });
     * @endcode
     */
    void use(const request_handler_t<T, G>& middleware) { routers[0]->use(middleware); }

protected:
    /**
     * @brief Serve static files from registered directories
     *
     * Attempts to serve a static file matching the request URI from the registered
     * static directories. Includes path sanitization for security and automatic
     * MIME type detection based on file extension.
     *
     * @param req Request object containing URI
     * @param res Response object for sending file content
     *
     * @note Directories are checked in registration order
     * @note Returns 404 if file not found in any directory
     * @note Exceptions are caught and handled via on_unhandled_exception
     */
    virtual void serve_static(std::shared_ptr<T> req, std::shared_ptr<G> res) {
        try {
            std::string uri = req->get_uri();
            std::string sanitized_path = shared::sanitize_path(uri);
            std::string file_path;

            /// If the file found in the registered static directories
            for (const auto& dir : static_directories) {
                file_path = dir + sanitized_path;
                if (std::ifstream(file_path)) {
                    break;
                }
            }
            /// No file, bad, return 404
            if (file_path.empty() || !std::ifstream(file_path)) {
                res->set_status(404, "Not Found");
                res->send_text("404 Not Found");
                return;
            }

            std::ifstream file(file_path);
            std::stringstream buffer;
            buffer << file.rdbuf();
            res->set_body(buffer.str());

            /// send the file to the browser
            res->set_content_type(
                shared::get_mime_type_from_extension(shared::get_file_extension_from_uri(uri)));
            res->set_status(200, "OK");
            res->send();
        } catch (const std::exception& e) {
            shared::logger::error("Error serving static file: " + std::string(e.what()));
            exception exp("Error serving static file", "INTERNAL_ERROR", "serve_static", 500,
                          "Internal Server Error");
            on_unhandled_exception(req, res, exp);
        }
    }

    /**
     * @brief Main request handler executing routing pipeline
     *
     * This is the core request processing method that orchestrates the entire
     * request handling flow:
     * -# Check if URI is for a static file
     * -# If static, serve the file
     * -# If dynamic, try each registered router
     * -# If no match found, call default handler (404)
     * -# Send response and manage connection (keep-alive or close)
     *
     * @param req Request object containing all request data
     * @param res Response object for building the response
     *
     * @note This function is called from worker threads
     * @note Exceptions are caught and handled appropriately
     * @note Response is sent automatically at the end
     * @note Connection handling respects HTTP keep-alive headers
     */
    virtual void request_handler(std::shared_ptr<T> req, std::shared_ptr<G> res) {
        try {
            bool handled =
                false;  // check if the route got matched with any of the registered routers
            if (is_uri_static(req->get_uri()))  // if static, serve the static file
            {
                serve_static(req, res);
                handled = true;
            } else {
                // check the routers if they can handle the request
                for (const auto& router : routers) {
                    if (router->handle_request(req, res)) {
                        handled = true;
                        break;
                    }
                }
            }

            if (!handled)
                handle_default_route(req, res);

            res->send();
            if (!req->keep_alive())
                res->end();

        } catch (const std::exception& e) {
            shared::logger::error("Error in request handler thread: " + std::string(e.what()));

            exception exp("Error in request handler thread", "INTERNAL_ERROR", "request_handler",
                          500, "Internal Server Error");

            on_unhandled_exception(req, res, exp);
        }

        res->send();
        if (!req->keep_alive())
            res->end();
    };

    /**
     * @brief HTTP server callback for incoming requests
     *
     * This method is called by the underlying HTTP server when a new request arrives.
     * It converts the low-level HTTP objects to high-level web objects and dispatches
     * the request to a worker thread for processing.
     *
     * @param request Low-level HTTP request object
     * @param response Low-level HTTP response object
     *
     * @note Validates HTTP method before processing
     * @note Returns 400 Bad Request for unknown HTTP methods
     * @note Enqueues request to worker pool for concurrent handling
     * @note Handles exceptions during enqueue operation
     */
    virtual void on_request_received(cppress::http::http_request& request,
                                     cppress::http::http_response& response) override {
        auto req = std::make_shared<T>(std::move(request));
        auto res = std::make_shared<G>(std::move(response));

        // If the pointers somehow was not created
        if (!res || !req) {
            shared::logger::error("Failed to create request/response objects");
            return;
        }

        // If an invalid HTTP method is received
        if (shared::unknown_method(req->get_method())) {
            shared::logger::error("Unknown HTTP method: " + req->get_method());

            // Send back bad Request
            res->set_status(400, "Bad Request");
            res->send_text("400 Bad Request: " + req->get_method());
            res->end();
            return;
        }
        try {
            // Enqueue the request handler for processing
            worker_pool.enqueue([this, req, res]() { request_handler(req, res); });
        } catch (web::exception& e)  // Unhandled exception
        {
            shared::logger::error("Error in request handler thread: " + std::string(e.what()));

            on_unhandled_exception(req, res, e);

            res->send();
            res->end();
        } catch (const std::exception& e)  // unexpected exception
        {
            shared::logger::error("Error in request handler thread: " + std::string(e.what()));

            web::exception exp("Error in request handler thread", "INTERNAL_ERROR",
                               "request_handler", 500, "Internal Server Error");

            on_unhandled_exception(req, res, exp);
            res->send();
            res->end();
        }
    };

    /**
     * @brief HTTP server callback for successful listen
     *
     * Called when the server successfully starts listening on the configured port.
     * Invokes the registered listen callback.
     */
    virtual void on_listen_success() override { this->listen_callback(); }

    /**
     * @brief HTTP server callback for exceptions
     *
     * Called when the HTTP server encounters an exception during operation.
     * Invokes the registered error callback for handling.
     *
     * @param e The exception that occurred
     */
    virtual void on_exception_occurred(const std::exception& e) override {
        this->error_callback(e);
    }

    /**
     * @brief HTTP server callback for header processing
     *
     * Called when HTTP headers are received but before body processing begins.
     * Allows for early request inspection, logging, header modification, or
     * connection termination based on header content.
     *
     * @param conn The connection object
     * @param headers The headers received as a multimap
     * @param method The HTTP method
     * @param uri The request URI
     * @param version The HTTP version
     * @param body The request body (may be partial or empty at this stage)
     *
     * @note Use close_connection(conn) to terminate the connection if needed
     * @note Only called if a callback was registered via use_headers_received()
     */
    virtual void on_headers_received(HEADER_RECEIVED_PARAMS) override {
        if (headers_callback)
            headers_callback(conn, headers, method, uri, version, body);
    }

    /**
     * @brief Handle unhandled web exceptions
     *
     * Called when an exception occurs during request processing and is not caught
     * by route handlers. If a custom error callback is registered, it is invoked;
     * otherwise, a default 500 error response is sent.
     *
     * @param req The HTTP request that caused the exception
     * @param res The HTTP response object for sending error response
     * @param e The exception that occurred
     *
     * @note Default behavior: sets 500 status, sends "Internal Server Error", logs error
     * @note Custom callback can be set via use_error() method
     * @note Always closes the connection after error response
     */
    virtual void on_unhandled_exception(std::shared_ptr<T> req, std::shared_ptr<G> res,
                                        exception& e) {
        if (unhandled_exception_callback) {
            unhandled_exception_callback(req, res, e);
            return;
        }
        res->set_status(e.get_status_code(), e.get_status_message());
        res->send_text("Internal Server Error");
        shared::logger::error("Unhandled Web exception: " + std::string(e.what()));
        res->end();
    }
};

}  // namespace cppress::web