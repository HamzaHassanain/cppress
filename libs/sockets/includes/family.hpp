/**
 * @file family.hpp
 * @brief Type-safe address family wrapper with validation.
 *
 * This file provides the family class, a validated wrapper around socket address
 * family constants (AF_INET, AF_INET6). It ensures only supported address families
 * are used and prevents accidental misuse of raw integer values in network operations.
 * **Creating Address Families:**
 * @code
 * #include "family.hpp"
 * using namespace cppress::sockets;
 *
 * // IPv4 (most common)
 * family ipv4(IPV4);  // IPV4 = AF_INET
 *
 * // IPv6
 * family ipv6(IPV6);  // IPV6 = AF_INET6
 *
 * // Default construction (IPv4)
 * family default_family;  // Defaults to IPv4
 *
 * // Invalid family
 * try {
 *     family invalid(999);  // Throws socket_exception
 * } catch (const socket_exception& e) {
 *     std::cerr << "Invalid family: " << e.what() << "\n";
 * }
 * @endcode
 *
 * **Accessing Family Value:**
 * @code
 * family f(IPV4);
 *
 * //
 * int family_id = f.value();
 * std::cout << "Family: " << family_id << "\n";
 *
 * // Implicit conversion
 * int id = f;
 *
 * // Use in system calls
 * int sock = socket(f, SOCK_STREAM, 0);
 * @endcode
 *
 * **Integration with socket_address:**
 * @code
 * #include "socket_address.hpp"
 *
 * // IPv4 address
 * socket_address ipv4_addr(
 *     port(8080),
 *     ip_address("192.168.1.1"),
 *     family(IPV4)
 * );
 *
 * // IPv6 address
 * socket_address ipv6_addr(
 *     port(8080),
 *     ip_address("::1"),
 *     family(IPV6)
 * );
 *
 * // Extract family from socket address
 * family f = ipv4_addr.family();
 * if (f == family(IPV4)) {
 *     std::cout << "IPv4 address\n";
 * }
 * @endcode
 *
 *
 * **Socket Creation with Family:**
 * @code
 * family f(IPV4);
 *
 * // Create TCP socket
 * int tcp_sock = socket(f.value(), SOCK_STREAM, 0);
 *
 * // Create UDP socket
 * int udp_sock = socket(f.value(), SOCK_DGRAM, 0);
 *
 * // Using implicit conversion
 * int sock = socket(f, SOCK_STREAM, 0);
 * @endcode
 *
 * **Determining Address Family:**
 * @code
 * void process_address(const socket_address& addr) {
 *     family f = addr.family();
 *
 *     if (f == family(IPV4)) {
 *         std::cout << "Processing IPv4 address\n";
 *         // IPv4-specific logic
 *     } else if (f == family(IPV6)) {
 *         std::cout << "Processing IPv6 address\n";
 *         // IPv6-specific logic
 *     }
 * }
 * @endcode

 * @section integration Integration with Sockets Library
 * The family class integrates with other cppress::sockets components:
 * - Component of socket_address (IP/port/family triplet)
 * - Used by socket creation (socket(), bind(), connect())
 * - Determines sockaddr structure type (sockaddr_in vs sockaddr_in6)
 * - Validated automatically at construction
 * - Used by utility functions for address conversion
 *

 * @author Hamza Mohammed Hassanain
 * @version 1.0
 */

#pragma once

#include <algorithm>
#include <ostream>
#include <string>
#include <vector>

#include "exceptions.hpp"
#include "utilities.hpp"

namespace cppress::sockets {
/**
 * @brief Represents an address family for socket operations (IPv4, IPv6).
 *
 * This class provides type-safe wrapper around socket address family constants,
 * preventing accidental misuse of raw integer values. It validates that only
 * supported address families (IPv4, IPv6) are used.
 *
 * @note Uses explicit constructors to prevent implicit conversions for type safety.
 *
 * Example usage:
 * @code
 * family ipv4_family(IPV4);
 * family ipv6_family(IPV6);
 * socket_address addr(ip_address("127.0.0.1"), port(8080), ipv4_family);
 * @endcode
 */

class family {
private:
    /// Allowed address family values (IPv4 and IPv6)
    std::vector<int> allowed_families = {IPV4, IPV6};

    /// Current address family ID
    int family_id;

    /**
     * @brief Validates and sets the family ID.
     * @param id The address family ID to set
     * @throws cppress::sockets::socket_exception if the ID is not IPV4 or IPV6
     */
    void set_family_id(int id) {
        if (std::find(allowed_families.begin(), allowed_families.end(), id) !=
            allowed_families.end()) {
            family_id = id;
        } else {
            throw socket_exception("Invalid family ID. Allowed families are IPV4 and IPV6.",
                                   "InvalidFamilyID", __func__);
        }
    }

public:
    /**
     * @brief Default constructor - initializes to IPv4.
     *
     * Creates a family object with IPv4 as the default address family.
     * Marked explicit to prevent implicit conversions.
     */
    explicit family() : family_id(IPV4) {}

    /**
     * @brief Construct family with specific address family.
     * @param id Address family constant (IPV4 or IPV6)
     * @throws std::invalid_argument if id is not a valid address family
     *
     * Example:
     * @code
     * family ipv4(IPV4);  // Valid
     * family ipv6(IPV6);  // Valid
     * // family invalid(999);  // Throws exception
     * @endcode
     */
    explicit family(int id) { set_family_id(id); }

    // Use compiler-generated copy operations
    family(const family& other) = default;
    family& operator=(const family& other) = default;

    // Use compiler-generated move operations
    family(family&& other) = default;             //  for move operations
    family& operator=(family&& other) = default;  //  for move assignment

    /**
     * @brief Get the raw address family value ().
     * @return Integer value representing the address family (AF_INET, AF_INET6, etc.)
     *
     * Returns the stored address family constant. This follows STL conventions
     * for value-type accessors like std::optional::value().
     */
    int value() const { return family_id; }

    /**
     * @brief Legacy accessor for backward compatibility.
     * @deprecated Use value() instead
     * @return Integer value representing the address family
     */
    [[deprecated("Use value() instead")]]
    int get() const {
        return value();
    }

    /**
     * @brief Equality comparison operator.
     * @param other Family object to compare with
     * @return true if both objects have the same family ID
     */
    bool operator==(const family& other) const { return family_id == other.family_id; }

    /**
     * @brief Inequality comparison operator.
     * @param other Family object to compare with
     * @return true if objects have different family IDs
     */
    bool operator!=(const family& other) const { return !(*this == other); }

    /**
     * @brief Less-than comparison operator for ordering.
     * @param other Family object to compare with
     * @return true if this family's ID is less than other's ID
     */
    bool operator<(const family& other) const { return family_id < other.family_id; }

    /**
     * @brief Stream insertion operator for output.
     * @param os Output stream
     * @param f Family object to output
     * @return Reference to the output stream
     */
    friend std::ostream& operator<<(std::ostream& os, const family& f) {
        os << f.family_id;
        return os;
    }
    /**
     * @brief Get the IPv4 address family constant.
     * @return int  the IPv4 address family constant
     */
    static family ipv4() { return family(IPV4); }
    static family ipv6() { return family(IPV6); }

    /**
     * @brief explicit conversion numeric types
     */
    int to_int() const noexcept { return family_id; }
    /**
     * @brief explicit conversion numeric types
     */
    long to_long() const noexcept { return static_cast<long>(family_id); }
    /**
     * @brief explicit conversion numeric types
     */
    double to_double() const noexcept { return static_cast<double>(family_id); }

    /// Default destructor
    ~family() = default;
};
}  // namespace cppress::sockets