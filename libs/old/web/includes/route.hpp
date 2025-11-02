/**
 * @file route.hpp
 * @brief Route definition and matching for web request handling
 *
 * This file defines the route class, which represents a single route in the
 * cppress web framework with its associated HTTP method, path pattern, and handlers.
 *
 * @section route_features Key Features
 * @li HTTP method-based routing (GET, POST, PUT, DELETE, etc.)
 * @li Path pattern matching with parameter extraction
 * @li Support for parameterized routes (e.g., /users/:id)
 * @li Multiple handler support for middleware chains
 * @li Automatic path parameter population in request objects
 * @li Handler execution with exit code control flow
 *
 * @section route_usage Usage Example
 * @code{.cpp}
 * // Create a route for GET /users/:id
 * auto userRoute = std::make_shared<route<>>(
 *     "GET",
 *     "/users/:id",
 *     {
 *         [](auto req, auto res) {
 *             auto params = req->get_path_params();
 *             std::string id = params["id"];
 *             res->send_json("{\"id\": \"" + id + "\"}");
 *             return exit_code::EXIT;
 *         }
 *     }
 * );
 *
 * // Add to router
 * router->add_route(userRoute);
 * @endcode
 *
 * @section route_return_values Handler Return Values
 * @li exit_code::EXIT: Stop processing, finalize response
 * @li exit_code::CONTINUE: Continue to next handler
 * @li exit_code::_ERROR: Indicate error condition
 *
 * @note Routes are typically created and managed by router objects
 * @note Path parameters are automatically extracted and set in request
 *
 * @author cppress team
 * @version 1.0
 */

#pragma once
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

#include "exceptions.hpp"
#include "types.hpp"
#include "utilities.hpp"

namespace cppress::web {
template <typename T, typename G>
class router;  // Forward declaration

/**
 * @brief Template class representing a web route with request handlers.
 *
 * This class encapsulates a web route definition that consists of an HTTP method,
 * a path expression (which may include wildcards or parameters), and a collection
 * of request handlers. It provides the foundation for URL routing in web applications
 * by matching incoming requests against route patterns and executing associated handlers.
 *
 * The class is templated to support custom request and response types while maintaining
 * type safety through compile-time checks.
 *
 * Routes can handle complex path patterns including:
 * - Static paths: "/api/users"
 * - Parameterized paths: "/api/users/:id"
 * - With Query parameters: "/api/users?id=123"
 * - Multiple handlers for middleware chains
 *
 * @tparam T Type for request objects (must derive from request)
 * @tparam G Type for response objects (must derive from response)

 */
template <typename T = request, typename G = response>
class route {
protected:
    /// HTTP method for this route (GET, POST, PUT, DELETE, etc.)
    std::string method;

    /// Path expression/pattern for route matching (may include parameters)
    std::string expression;

    /// Collection of request handlers executed in sequence for this route
    std::vector<request_handler_t<T, G>> handlers;

public:
    /// Allow router to access private members
    friend class router<T, G>;

    /**
     * @brief Construct a web route with method, path expression, and handlers.
     * @param method HTTP method this route responds to (e.g., "GET", "POST", "PUT", "DELETE")
     * @param expression Path pattern for route matching (e.g., "/api/users/:id")
     * @param handlers Vector of request handlers to execute for this route
     *
     * Creates a new web route that will match requests with the specified HTTP method
     * and path pattern. The handlers are executed in sequence when a request matches
     * this route, allowing for middleware chains and complex request processing.
     *
     * The constructor performs compile-time type checking to ensure T and
     * G derive from the base request and response classes respectively.
     * It also validates that at least one handler is provided.
     *
     * @throws std::invalid_argument if no handlers are provided
     */
    route(const std::string& method, const std::string& expression,
          const std::vector<request_handler_t<T, G>>& handlers)
        : method(method), expression(expression), handlers(std::move(handlers)) {
        static_assert(std::is_base_of<request, T>::value, "T must derive from request");
        static_assert(std::is_base_of<response, G>::value, "G must derive from response");
        if (this->handlers.size() == 0) {
            throw std::invalid_argument("At least one handler must be provided");
        }
    }

    /**
     * @brief Get the path expression/pattern for this route.
     * @return String containing the path pattern used for route matching
     *
     * Returns the path expression that defines what URLs this route will match.
     * This includes any parameter placeholders or wildcard patterns that were
     * specified when the route was created.
     */
    virtual std::string get_path() const { return expression; }

    /**
     * @brief Get the HTTP method for this route.
     * @return String containing the HTTP method (GET, POST, PUT, DELETE, etc.)
     *
     * Returns the HTTP method that this route is configured to handle.
     * Only requests with matching methods will be processed by this route.
     */
    virtual std::string get_method() const { return method; }

    /**
     * @brief Check if this route matches the given method and path.
     * @param request Shared pointer to the request object
     * @return True if the route matches, false otherwise
     *
     * Performs route matching by comparing the provided HTTP method and path
     * against this route's configured method and path expression, then sets the path parameters if
     * existing ones are found.
     *
     * @note This function is called by the router class to determine
     * if a request matches this route.
     */
    virtual bool match(std::shared_ptr<T> request) const {
        auto [matched, path_params] = match_path(this->expression, request->get_path());
        if (matched) {
            request->set_path_params(path_params);
        }
        return this->method == request->get_method() && matched;
    }

    /**
     * @brief Match request against routes and execute matching route handlers.
     * @param request Shared pointer to the request object
     * @param response Shared pointer to the response object
     * @return exit_code indicating the result of route processing
     *
     * Executes all handlers associated with that route in the order they were registered.
     *
     * Route handlers can return:
     * - EXIT: Stop processing and finalize the response
     * - ERROR: Indicate an error condition
     *
     *@note This function is called by the router class if a matching route is found.
     *@note This is a const member function, meaning it does not modify the state of the route
     * instance.
     */
    virtual exit_code handle_request(std::shared_ptr<T> request,
                                     std::shared_ptr<G> response) const {
        for (const auto& handler : handlers) {
            auto resp = handler(request, response);
            if (resp == exit_code::EXIT) {
                return exit_code::EXIT;
            } else if (resp == exit_code::_ERROR) {
                return exit_code::_ERROR;
            } else if (resp == exit_code::CONTINUE) {
                continue;
            } else {
                throw std::runtime_error(
                    "Invalid route handler, return value must of  cppress::socketsexit_code\n");
            }
        }

        return exit_code::EXIT;
    }

    /// Default virtual destructor for proper cleanup in inheritance hierarchies
    ~route() = default;
};
}  // namespace cppress::web