/**
 * @file exceptions.hpp
 * @brief Web framework exception handling with HTTP status code support
 *
 * This file defines the web exception class for the cppress web framework.
 * It extends the base socket exception to provide HTTP-aware error handling
 * with proper status codes and messages for client responses.
 *
 * @section exception_features Key Features
 * @li HTTP status code integration (400, 404, 500, etc.)
 * @li Custom status messages for better error descriptions
 * @li Multiple constructors for different use cases
 * @li Thread-safe error information retrieval
 * @li Formatted error messages including HTTP status details
 *
 * @section exception_usage Usage Example
 * @code{.cpp}
 * // Throw a 404 Not Found exception
 * throw cppress::web::exception("Resource not found", 404, "Not Found");
 *
 * // Catch and handle web exceptions
 * try {
 *     // ... web operation
 * } catch (const cppress::web::exception& e) {
 *     response->set_status(e.get_status_code(), e.get_status_message());
 *     response->send_text(e.what());
 * }
 * @endcode
 *
 * @section common_status_codes Common HTTP Status Codes
 * @li 400: Bad Request - Client sent invalid data
 * @li 401: Unauthorized - Authentication required
 * @li 403: Forbidden - Access denied
 * @li 404: Not Found - Resource doesn't exist
 * @li 500: Internal Server Error - Server-side error
 * @li 503: Service Unavailable - Server overloaded/down
 *
 * @author cppress team
 * @version 1.0
 */

#pragma once
#include "sockets/includes.hpp"

namespace cppress::web {
/**
 * @brief Web-specific exception class for HTTP-related errors.
 *
 * This class extends the base socket_exception to provide web framework-specific
 * error handling with HTTP status codes and status messages. It is designed to
 * handle various web-related exceptions that can occur during HTTP request
 * processing, routing, and response generation.
 *
 * The class maintains HTTP status codes and messages to provide proper HTTP
 * error responses to clients while maintaining detailed error information
 * for debugging and logging purposes.

 */
class exception : public cppress::sockets::socket_exception {
    int status_code = 500;  ///< HTTP status code (default: 500 Internal Server Error)
    std::string status_message = "Internal Server Error";  ///< HTTP status message
    std::string _formatted_message;                        ///< Cached formatted message for what()
public:
    /**
     * @brief Construct web exception with error message.
     * @param message Descriptive error message explaining the web operation failure
     *
     * Creates a web exception with default HTTP 500 status code and "Internal Server Error"
     * message. Uses default type "WEB_EXCEPTION" and function "function".
     */
    explicit exception(const std::string& message)
        : socket_exception(message, "WEB_EXCEPTION", "function"), _formatted_message(message) {}

    /**
     * @brief Construct web exception with custom HTTP status.
     * @param message Descriptive error message explaining the web operation failure
     * @param status_code HTTP status code for the error response
     * @param status_message HTTP status message corresponding to the status code
     *
     * Creates a web exception with custom HTTP status information for proper client response.
     */
    explicit exception(const std::string& message, int status_code,
                       const std::string& status_message)
        : socket_exception(message, "", ""),
          status_code(status_code),
          status_message(status_message),
          _formatted_message(message) {}

    /**
     * @brief Construct web exception with type and function information.
     * @param message Descriptive error message explaining the web operation failure
     * @param type Type identifier for the exception
     * @param function Name of the function that threw the exception
     *
     * Creates a web exception with custom type and function information while maintaining
     * default HTTP 500 status code.
     */
    explicit exception(const std::string& message, const std::string& type,
                       const std::string& function)
        : socket_exception(message, type, function), _formatted_message(message) {}

    /**
     * @brief Construct web exception with full customization.
     * @param message Descriptive error message explaining the web operation failure
     * @param type Type identifier for the exception
     * @param function Name of the function that threw the exception
     * @param status_code HTTP status code for the error response
     *
     * Creates a web exception with complete customization of all parameters including
     * HTTP status code while using default status message.
     */
    explicit exception(const std::string& message, const std::string& type,
                       const std::string& function, int status_code = 500,
                       std::string status_message = "Internal Server Error")
        : socket_exception(message, type, function),
          status_code(status_code),
          status_message(status_message),
          _formatted_message(message) {}

    /**
     * @brief Get the HTTP status message.
     * @return String containing the HTTP status message
     * @note Thread-safe and returns the status message associated with the HTTP error
     *
     * Returns the HTTP status message that describes the error condition in human-readable
     * format. This is typically used in HTTP response headers and error pages.
     */
    std::string get_status_message() const noexcept { return status_message; }

    /**
     * @brief Get the HTTP status code.
     * @return Integer representing the HTTP status code
     * @note Thread-safe and returns the numeric HTTP status code
     *
     * Returns the HTTP status code that should be sent in the HTTP response.
     * Common codes include 400 (Bad Request), 404 (Not Found), 500 (Internal Server Error), etc.
     */
    int get_status_code() const noexcept { return status_code; }

    /**
     * @brief Get the formatted error message string.
     * @return C-style string containing the formatted error message with HTTP status information
     * @note Thread-safe and returns a persistent pointer to the formatted message
     *
     * Overrides the base class what() method to include HTTP status code and message
     * in the formatted error output. The format includes status code, status message,
     * and the underlying socket exception details.
     */
    std::string what() noexcept override {
        std::string formatted_message = "Web Exception [" + std::to_string(status_code) + " - " +
                                        status_message + "]: " + _formatted_message;

        return formatted_message;
    }
};

}  // namespace cppress::web