/**
 * @file ip_address.hpp
 * @brief Type-safe IP address wrapper with STL-style interface.
 *
 * This file provides the ip_address class, a lightweight, type-safe wrapper around
 * IP address strings for both IPv4 and IPv6 addresses. It provides convenient
 * comparison operations and STL-compatible interfaces for use in network programming.
 *
 * @section usage Common Usage Patterns
 *
 * **Creating IP Addresses:**
 * @code
 * #include "ip_address.hpp"
 * using namespace cppress::sockets;
 *
 * // IPv4 addresses
 * ip_address localhost("127.0.0.1");
 * ip_address server("192.168.1.100");
 * ip_address wildcard("0.0.0.0");
 *
 * // IPv6 addresses
 * ip_address ipv6_localhost("::1");
 * ip_address ipv6_addr("2001:0db8:85a3::8a2e:0370:7334");
 * ip_address ipv6_wildcard("::");
 *
 * // Default construction (empty)
 * ip_address empty;
 * @endcode
 *
 * **Copy and Move:**
 * @code
 * // Copy (efficient for small strings)
 * ip_address addr1("192.168.1.1");
 * ip_address addr2 = addr1;
 *
 * // Move (more efficient)
 * ip_address addr3 = std::move(addr1);
 *
 * // Function returns
 * ip_address get_server_ip() {
 *     return ip_address("192.168.1.100");
 * }
 * ip_address server = get_server_ip();  // RVO or move
 * @endcode
 *
 * **Integration with socket_address:**
 * @code
 * #include "socket_address.hpp"
 *
 * // Create socket address with IP
 * ip_address server_ip("192.168.1.100");
 * socket_address addr(port(8080), server_ip);
 *
 * // Extract IP from socket address
 * ip_address extracted = addr.address();
 * std::cout << "Server at " << extracted << "\n";
 * @endcode
 *
 * @section validation Validation Notes
 *
 * **Important**: This class does NOT validate IP address format. It stores
 * whatever string is provided. Validation should be performed by:
 * - Higher-level classes (socket_address)
 * - Utility functions (convert_ip_address_to_network_order)
 * - System calls (inet_pton will fail on invalid addresses)
 *
 * Example of what is accepted (but may fail later):
 * @code
 * ip_address invalid1("not.an.ip");     // Accepted, but invalid
 * ip_address invalid2("999.999.999.999"); // Accepted, out of range
 * ip_address invalid3("");              // Accepted, empty string
 * @endcode
 *
 * @section integration Integration with Sockets Library
 * The ip_address class integrates with other cppress::sockets components:
 * - Component of socket_address (IP/port/family triplet)
 * - Used by utilities for network byte order conversion
 * - Extracted from sockaddr structures
 * - Provides string representation for display and logging
 *
 *
 *
 * @author Hamza Mohammed Hassanain
 * @version 1.0
 */

#pragma once

#include <ostream>
#include <string>

namespace cppress::sockets {
/**
 * @brief Represents an IP address (IPv4 or IPv6) for network operations.
 *
 * This class provides a type-safe wrapper around IP address strings, offering
 * validation and convenient comparison operations. It can handle both IPv4
 * addresses (e.g., "192.168.1.1") and IPv6 addresses (e.g., "::1", "2001:db8::1").
 *
 * The class stores the IP address as a string representation, which is
 * platform-independent and suitable for both display and network operations.
 * It provides standard comparison operators for use in containers and sorting.
 *
 * @note Uses explicit constructors to prevent implicit conversions for type safety.
 * @note The class does not perform validation of IP address format - it stores
 *       whatever string is provided. Validation should be performed by the caller
 *       or higher-level network classes.
 *
 * @endcode
 */
class ip_address {
private:
    /// String representation of the IP address
    std::string address;

public:
    /**
     * @brief Default constructor - creates an empty IP address.
     *
     * Creates an ip_address object with an empty string. This is useful for
     * creating objects that will be assigned values later or for representing
     * an unspecified address.
     *
     * Marked explicit to prevent implicit conversions.
     */
    explicit ip_address() = default;

    /**
     * @brief Construct IP address from string representation.
     * @param address String containing the IP address
     *
     * Creates an ip_address object from a string representation of an IP address.
     * The string can contain IPv4 addresses (dotted decimal notation) or IPv6
     * addresses (colon-hexadecimal notation).
     *
     * @note This constructor does not validate the IP address format.
     *       Invalid strings will be stored as-is, which may cause issues
     *       in network operations.
     */
    explicit ip_address(const std::string& address) : address(address) {}

    // Copy operations - Safe and efficient for string-based class
    ip_address(const ip_address&) = default;
    ip_address& operator=(const ip_address&) = default;

    // Move operations - Efficient for string transfer
    ip_address(ip_address&&) = default;
    ip_address& operator=(ip_address&&) = default;

    /**
     * @brief Get the IP address string (STL-style accessor).
     * @return Const reference to the IP address string
     *
     * Returns a const reference to the internal string representation
     * of the IP address. This avoids copying and allows efficient
     * access to the address value.
     */
    const std::string& string() const { return address; }

    /**
     * @brief Legacy accessor for backward compatibility.
     * @deprecated Use string() instead
     * @return Const reference to the IP address string
     */
    [[deprecated("Use string() instead")]]
    const std::string& get() const {
        return string();
    }

    /**
     * @brief Explicit conversion to string for convenience.
     * @return Copy of the IP address string
     *
     * Allows using ip_address objects directly where strings are expected:
     */
    std::string to_string() const { return address; }

    /**
     * @brief Equality comparison operator.
     * @param other IP address object to compare with
     * @return true if both objects contain the same IP address string
     */
    bool operator==(const ip_address& other) const { return address == other.address; }

    /**
     * @brief Inequality comparison operator.
     * @param other IP address object to compare with
     * @return true if objects contain different IP address strings
     */
    bool operator!=(const ip_address& other) const { return !(*this == other); }

    /**
     * @brief Less-than comparison operator for ordering.
     * @param other IP address object to compare with
     * @return true if this address string is lexicographically less than other's
     */
    bool operator<(const ip_address& other) const { return address < other.address; }

    /**
     * @brief Stream insertion operator for output.
     * @param os Output stream
     * @param ip IP address object to output
     * @return Reference to the output stream
     */
    friend std::ostream& operator<<(std::ostream& os, const ip_address& ip) {
        os << ip.address;
        return os;
    }

    /// Default destructor
    ~ip_address() = default;
};
}  // namespace cppress::sockets