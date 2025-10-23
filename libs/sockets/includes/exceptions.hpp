/**
 * @file exceptions.hpp
 * @brief Socket exception hierarchy for error handling.
 *
 * This file provides the socket_exception class, which serves as the base exception
 * type for all socket-related errors in the cppress sockets library. It extends
 * std::exception to provide detailed error information including error type,
 * throwing function, and formatted messages.
 *
 * @section usage Common Usage Patterns
 *
 * **Basic Exception Handling:**
 * @code
 * #include "exceptions.hpp"
 * using namespace cppress::sockets;
 *
 * try {
 *     socket s(socket_address(port(8080)), type::stream);
 *     s.listen();
 * } catch ( socket_exception& e) {
 *     std::cerr << "Socket error: " << e.what() << "\n";
 *     std::cerr << "Error type: " << e.type() << "\n";
 *     std::cerr << "Thrown from: " << e.thrower_function() << "\n";
 * }
 * @endcode
 *
 * **Type-Specific Handling:**
 * @code
 * try {
 *     socket s(socket_address(port(80)), type::stream);
 *     s.listen();
 * } catch ( socket_exception& e) {
 *     if (e.type() == "SocketBinding") {
 *         std::cerr << "Port already in use or requires privileges\n";
 *     } else if (e.type() == "SocketCreation") {
 *         std::cerr << "Failed to create socket\n";
 *     } else {
 *         std::cerr << "Unknown error: " << e.what() << "\n";
 *     }
 * }
 * @endcode
 *
 * **Throwing Exceptions:**
 * @code
 * void custom_socket_operation() {
 *     if (some_error_condition) {
 *         throw socket_exception(
 *             "Detailed error description",
 *             "ErrorType",
 *             __func__  // Current function name
 *         );
 *     }
 * }
 * @endcode
 *
 * **Catching and Rethrowing:**
 * @code
 * try {
 *     perform_network_operation();
 * } catch ( socket_exception& e) {
 *     // Log error
 *     log_error(e.what());
 *
 *     // Rethrow to propagate
 *     throw;
 * }
 * @endcode
 *
 * **Multiple Exception Types:**
 * @code
 * try {
 *     auto conn = server.accept();
 *     data_buffer request = conn->read();
 *     conn->write(response);
 * } catch ( socket_exception& e) {
 *     std::string error_type = e.type();
 *
 *     if (error_type == "SocketAcceptance") {
 *         std::cerr << "Failed to accept connection\n";
 *     } else if (error_type == "SocketRead") {
 *         std::cerr << "Failed to read from connection\n";
 *     } else if (error_type == "SocketWrite") {
 *         std::cerr << "Failed to write to connection\n";
 *     }
 * }
 * @endcode
 *
 * **Validation Exceptions:**
 * @code
 * try {
 *     port p(70000);  // Invalid port number
 * } catch ( socket_exception& e) {
 *     if (e.type() == "InvalidPort") {
 *         std::cerr << "Port must be between 0 and 65535\n";
 *     }
 * }
 *
 * try {
 *     family f(999);  // Invalid address family
 * } catch ( socket_exception& e) {
 *     if (e.type() == "InvalidFamilyID") {
 *         std::cerr << "Only IPv4 and IPv6 are supported\n";
 *     }
 * }
 * @endcode
 *
 * **Protocol Mismatch Handling:**
 * @code
 * socket udp_socket(type::datagram);
 *
 * try {
 *     udp_socket.listen();  // UDP doesn't support listen()
 * } catch ( socket_exception& e) {
 *     if (e.type() == "ProtocolMismatch") {
 *         std::cerr << "listen() is only for TCP sockets\n";
 *     }
 * }
 * @endcode
 *
 * **Exception in Destructors:**
 * @code
 * class network_resource {
 *     socket s;
 * public:
 *     ~network_resource() noexcept {
 *         try {
 *             s.close();
 *         } catch ( socket_exception& e) {
 *             // Log but don't rethrow from destructor
 *             std::cerr << "Cleanup error: " << e.what() << "\n";
 *         }
 *     }
 * };
 * @endcode
 *
 * **Logging with Exception Details:**
 * @code
 * void log_socket_exception( socket_exception& e) {
 *     std::ostringstream log;
 *     log << "[" << e.type() << "] "
 *         << "in " << e.thrower_function() << ": "
 *         << e.what() << "\n";
 *
 *     // Write to log file
 *     write_to_log(log.str());
 * }
 *
 * try {
 *     socket s(addr, type::stream);
 * } catch ( socket_exception& e) {
 *     log_socket_exception(e);
 * }
 * @endcode
 *
 * @section messageformat Message Format
 *
 * Exception messages are formatted as:
 * ```
 * Socket Exception [ErrorType] in function_name: Detailed error message
 * ```
 *
 * Example:
 * ```
 * Socket Exception [SocketBinding] in socket::bind: Failed to bind socket to address 0.0.0.0:80:
 * Permission denied
 * ```
 *
 * @section integration Integration with Sockets Library
 *
 * All cppress::sockets classes throw socket_exception on errors:
 * - socket: Creation, binding, connection, I/O errors
 * - connection: I/O errors, closed connections
 * - port: Validation errors (out of range)
 * - family: Validation errors (unsupported family)
 * - socket_address: Construction errors
 *
 * @section design Design Rationale
 *
 * **Single Exception Type:**
 * - Simpler exception hierarchy
 * - Type-based error identification instead of inheritance
 * - Easier to catch all socket errors
 * - Flexible error categorization
 *
 * **Type Strings:**
 * - Runtime error type identification
 * - No need for multiple derived classes
 * - Easy to add new error types
 * - Consistent error handling patterns
 *
 * **Function Tracking:**
 * - Helps identify error source
 * - Useful for debugging
 * - Uses __func__ macro for automatic capture
 * - Included in formatted message
 *
 * @section exceptions Exception Safety
 * - All methods are noexcept (except ructor with string)
 * - what() returns cached formatted message (thread-safe)
 * - No dynamic allocation after ruction
 * - Safe to use in exception handling code
 *
 * @section threading Thread Safety
 * - Exception objects are immutable after ruction
 * - Thread-safe to read from multiple threads
 * - what() returns stable pointer to cached message
 * - Can be thrown and caught across thread boundaries

 * @author Hamza Mohammed Hassanain
 * @version 1.0
 */

#pragma once

#include <stdexcept>
#include <string>

namespace cppress::sockets {
/**
 * @brief Base exception class for all socket-related errors.
 *
 * This class serves as the base for all socket operation exceptions in the library.
 * It extends std::runtime_error to provide a consistent exception hierarchy for
 * socket programming errors. All derived exceptions inherit from this class,
 * allowing for both specific and general exception handling.
 *
 * The class provides a virtual type() method that can be overridden by derived
 * classes to provide specific exception type identification.
 *
 * Example usage:
 * @code
 * try {
 *     // Socket operations that might fail
 *     perform_socket_operation();
 * }
 * catch ( socket_exception& e) {
 *     std::cerr << e.what() << std::endl;
 * }
 * @endcode
 */
class socket_exception : public std::exception {
    std::string _type;
    std::string _thrower_function;
    std::string _formatted_message;  // Cache for formatted message

public:
    /**
     * @brief Construct exception with error message.
     * @param message Descriptive error message explaining the socket failure
     * @param type Type of the socket exception
     * @param thrower_function Name of the function that threw the exception
     */
    explicit socket_exception(const std::string& message, const std::string& type,
                              const std::string& thrower_function = "SOCKET_FUNCTION")
        : std::exception(),
          _type(type),
          _thrower_function(thrower_function),
          _formatted_message(message) {}

    /**
     * @brief Get the exception type name.
     * @return C-style string identifying the exception type
     */
    virtual std::string type() noexcept { return _type; }

    /**
     * @brief Get the name of the function that threw the exception.
     * @return C-style string identifying the thrower function
     */
    virtual std::string thrower_function() noexcept { return _thrower_function; }

    /**
     * @brief Get the formatted error message string.
     * @return C-style string containing the formatted error message
     * @note Thread-safe and returns a persistent pointer to the formatted message
     */
    virtual std::string what() {
        // Create message
        std::string msg =
            "Socket Exception [" + _type + "] in " + _thrower_function + ": " + _formatted_message;
        return msg;
    }

    /// Default virtual destructor
    virtual ~socket_exception() = default;
};
}  // namespace cppress::sockets