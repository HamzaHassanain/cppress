/**
 * @file port.hpp
 * @brief Type-safe network port wrapper with validation.
 *
 * This file provides the port class, a validated wrapper around port numbers that
 * ensures only valid port values (0-65535) are used in network operations. It
 * prevents common networking errors by validating port numbers at construction time.
 *
 * @section usage Common Usage Patterns
 *
 * **Creating Ports:**
 * @code
 * #include "port.hpp"
 * using namespace cppress::sockets;
 *
 * // Valid ports
 * port http(80);
 * port https(443);
 * port custom(8080);
 * port ephemeral(49152);
 *
 * // Port validation at construction
 * try {
 *     port invalid(70000);  // Throws socket_exception
 * } catch (const socket_exception& e) {
 *     std::cerr << "Invalid port: " << e.what() << "\n";
 * }
 *
 * // Default construction
 * port p;  // Uninitialized, must assign before use
 * p = port(3000);
 * @endcode
 *
 * **Common Port Numbers:**
 * @code
 * // Well-known ports (0-1023)
 * port ftp_data(20);
 * port ftp_control(21);
 * port ssh(22);
 * port telnet(23);
 * port smtp(25);
 * port dns(53);
 * port http(80);
 * port pop3(110);
 * port https(443);
 *
 * // Registered ports (1024-49151)
 * port mysql(3306);
 * port postgresql(5432);
 * port mongodb(27017);
 * port redis(6379);
 *
 * // Dynamic/private ports (49152-65535)
 * port ephemeral(50000);
 * @endcode
 *
 * @section integration Integration with Sockets Library
 * The port class integrates with other cppress::sockets components:
 * - Component of socket_address (IP/port/family triplet)
 * - Used by socket class for binding and connecting
 * - Validated automatically at construction
 * - Provides value() for network byte order conversion
 * - Used by utility functions (is_free_port, get_random_free_port)
 *
 * @author Hamza Mohammed Hassanain
 * @version 1.0
 */

#pragma once

#include <ostream>
#include <stdexcept>

#include "exceptions.hpp"
#include "utilities.hpp"
namespace cppress::sockets {
/**
 * @brief Represents a network port number with validation.
 *
 * This class provides type-safe wrapper around port numbers, ensuring
 * only valid port values (0-65535) are used. It prevents common networking
 * errors by validating port numbers at construction time.
 *
 * @note Uses explicit constructors to prevent implicit conversions.
 * @note The class does not check if the port number is already in use, there is a separate
 * mechanism for that.
 */
class port {
private:
    /// Port number (0-65535)
    int port_id;

    /**
     * @brief Validates and sets the port ID.
     * @param id Port number to validate and set
     * @throws cppress::sockets::invalid_port_exception if port is not in range 0-65535
     */
    void set_port_id(int id) {
        if (id < cppress::sockets::MIN_PORT || id > cppress::sockets::MAX_PORT) {
            throw socket_exception("Port number must be in range 0-65535", "InvalidPort", __func__);
        }
        port_id = id;
    }

public:
    /**
     * @brief Default constructor - creates uninitialized port.
     *
     * Creates a port object with undefined value. Must be assigned
     * a valid port number before use.
     */
    explicit port() = default;

    /**
     * @brief Construct port with validation.
     * @param id Port number to use
     * @throws std::invalid_argument if port is not in valid range
     */
    explicit port(int id) { set_port_id(id); }

    // Copy and move operations
    port(const port&) = default;
    port(port&&) = default;
    port& operator=(const port&) = default;
    port& operator=(port&&) = default;

    /**
     * @brief Get the port number (STL-style accessor).
     * @return Port number as integer
     *
     * Returns the stored port number value. This follows STL conventions
     * for value-type accessors like std::optional::value().
     */
    int value() const { return port_id; }

    /**
     * @brief Legacy accessor for backward compatibility.
     * @deprecated Use value() instead
     * @return Port number as integer
     */
    [[deprecated("Use value() instead")]]
    int get() const {
        return value();
    }

    /**
     * @brief Implicit conversion to int for convenience.
     * @return Port number as integer
     *
     * Allows using port objects directly where integers are expected:
     * @code
     * port p(8080);
     * int port_num = p; // Implicit conversion
     * @endcode
     */
    operator int() const { return port_id; }

    /**
     * @brief Equality comparison operator.
     * @param other Port to compare with
     * @return true if port numbers are equal
     */
    bool operator==(const port& other) const { return port_id == other.port_id; }

    /**
     * @brief Inequality comparison operator.
     * @param other Port to compare with
     * @return true if port numbers are different
     */
    bool operator!=(const port& other) const { return !(*this == other); }

    /**
     * @brief Less-than comparison for ordering.
     * @param other Port to compare with
     * @return true if this port number is less than other's
     */
    bool operator<(const port& other) const { return port_id < other.port_id; }

    /**
     * @brief Stream insertion operator.
     * @param os Output stream
     * @param p Port object to output
     * @return Reference to output stream
     */
    friend std::ostream& operator<<(std::ostream& os, const port& p) {
        os << p.port_id;
        return os;
    }

    /// Default destructor
    ~port() = default;
};
}  // namespace cppress::sockets