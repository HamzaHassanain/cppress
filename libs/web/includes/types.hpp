/**
 * @file types.hpp
 * @brief Type definitions and aliases for the web framework
 *
 * This file defines common type aliases and enumerations used throughout
 * the cppress web framework for better code readability and maintainability.
 *
 * @section types_defined Defined Types
 *
 * @subsection exit_code_enum exit_code (enum class)
 * Controls the flow of request processing in handlers and middleware:
 * @li EXIT: Stop processing, send response and finalize
 * @li CONTINUE: Continue to next handler/middleware
 * @li _ERROR: Indicate an error condition
 *
 * @subsection callback_types Callback Types
 * @li request_handler_t: Handler for processing requests (routes/middleware)
 * @li unhandled_exception_callback_t: Callback for unhandled exceptions
 * @li listen_callback_t: Callback when server starts listening
 * @li error_callback_t: Callback for server errors
 * @li http_request_callback_t: Low-level HTTP request callback
 *
 * @section types_usage Usage Example
 * @code{.cpp}
 * // Define a request handler
 * request_handler_t<> myHandler = [](auto req, auto res) {
 *     res->send_text("Hello");
 *     return exit_code::EXIT;  // Stop processing
 * };
 *
 * // Define middleware that continues
 * request_handler_t<> logMiddleware = [](auto req, auto res) {
 *     std::cout << req->get_path() << std::endl;
 *     return exit_code::CONTINUE;  // Continue to next handler
 * };
 * @endcode
 *
 * @note These types are used throughout the web framework
 * @note Template parameters default to request and response
 *
 * @author cppress team
 * @version 1.0
 */

#pragma once
#include <functional>
#include <initializer_list>
#include <memory>

#include "exceptions.hpp"
#include "http/includes.hpp"
#include "request.hpp"
#include "response.hpp"

namespace cppress::web {
enum class exit_code { EXIT = 1, CONTINUE = 0, _ERROR = -1 };
using http_request_callback_t =
    std::function<void(cppress::http::http_request&, cppress::http::http_response&)>;
using listen_callback_t = std::function<void()>;
using error_callback_t = std::function<void(const std::exception&)>;

template <typename T = request, typename G = response>
using unhandled_exception_callback_t =
    std::function<void(std::shared_ptr<T>, std::shared_ptr<G>, const exception&)>;

template <typename T = request, typename G = response>
using request_handler_t = std::function<exit_code(std::shared_ptr<T>, std::shared_ptr<G>)>;

};  // namespace cppress::web