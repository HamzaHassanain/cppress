/**
 * @file socket_address.hpp
 * @brief Complete socket address composition with STL-style interface.
 *
 * This file provides the socket_address class, which combines IP address, port number,
 * and address family into a single, type-safe structure for network operations. It
 * manages the underlying system sockaddr structures and provides convenient access
 * to all components.
 *
 * @section usage Common Usage Patterns
 *
 * **Creating Socket Addresses:**
 * @code
 * #include "socket_address.hpp"
 * using namespace cppress::sockets;
 *
 * // IPv4 address with all components
 * socket_address addr1(
 *     port(8080),
 *     ip_address("192.168.1.100"),
 *     family(IPV4)
 * );
 *
 * // IPv4 with defaults (0.0.0.0, IPV4)
 * socket_address addr2(port(3000));
 *
 * // Wildcard address for server binding
 * socket_address server_addr(
 *     port(8080),
 *     ip_address("0.0.0.0"),
 *     family(IPV4)
 * );
 *
 * // IPv6 address
 * socket_address ipv6_addr(
 *     port(8080),
 *     ip_address("::1"),  // localhost
 *     family(IPV6)
 * );
 *
 * // From system sockaddr_storage
 * sockaddr_storage sys_addr;
 * // ... fill sys_addr from accept() or recvfrom() ...
 * socket_address addr3(sys_addr);
 * @endcode
 *
 *
 *
 * **Copy and Move Operations:**
 * @code
 * // Copy creates deep copy of sockaddr structure
 * socket_address addr1(port(8080), ip_address("127.0.0.1"));
 * socket_address addr2 = addr1;  // Deep copy
 *
 * // Move transfers ownership
 * socket_address addr3 = std::move(addr1);
 *
 * // Store in containers
 * std::vector<socket_address> addresses;
 * addresses.push_back(socket_address(port(80), ip_address("1.1.1.1")));
 * addresses.emplace_back(port(443), ip_address("8.8.8.8"));
 * @endcode
 *
 * @section integration Integration with Sockets Library
 * The socket_address class integrates with other cppress::sockets components:
 * - Composed of ip_address, port, and family classes
 * - Used by socket class for bind/connect/accept operations
 * - Stored in connection objects for endpoint information
 * - Provides raw sockaddr pointers for system calls
 * - Automatically handles network byte order conversion
 *
 *
 * @author Hamza Mohammed Hassanain
 * @version 1.0
 */

#pragma once

#include <memory>
#include <string>

#include "family.hpp"
#include "ip_address.hpp"
#include "port.hpp"

namespace cppress::sockets {
/**
 * @brief Represents a complete socket address combining IP, port, and address family.
 *
 * This class encapsulates all components needed for network socket operations:
 * IP address, port number, and address family (IPv4/IPv6). It manages the
 * underlying system sockaddr structures and provides type-safe access to
 * socket address components.
 *
 * @note Handles both IPv4 (sockaddr_in) and IPv6 (sockaddr_in6) addresses
 * @note Provides automatic conversion between host and network byte order
 */
class socket_address {
private:
    /// IP address component
    cppress::sockets::ip_address address_;

    /// Address family (IPv4 or IPv6)
    cppress::sockets::family family_;

    /// Port number component
    cppress::sockets::port port_;

    /// Underlying system socket address structure
    /// may be either sockaddr_in or sockaddr_in6
    std::shared_ptr<sockaddr> addr;

public:
    /**
     * @brief Default constructor - creates uninitialized socket address.
     */
    explicit socket_address() = default;

    /**
     * @brief Construct socket address from components.
     * @param port_id Port number
     * @param address IP address default 0.0.0.0
     * @param family_id Address family (IPv4/IPv6) default AF_INET (IPv4)
     *
     * Creates a socket address by combining IP address, port, and family.
     * Automatically creates appropriate sockaddr structure internally.
     */
    explicit socket_address(
        const cppress::sockets::port& port_id,
        const cppress::sockets::ip_address& address = cppress::sockets::ip_address("0.0.0.0"),
        const cppress::sockets::family& family_id = cppress::sockets::family(IPV4));

    /**
     * @brief Construct socket address from components.
     * @param address IP address
     * @param port_id Port number
     * @param family_id Address family (IPv4/IPv6) default AF_INET (IPv4)
     *
     * Creates a socket address by combining IP address, port, and family.
     * Automatically creates appropriate sockaddr structure internally.
     */
    explicit socket_address(
        const cppress::sockets::ip_address& address, const cppress::sockets::port& port_id,
        const cppress::sockets::family& family_id = cppress::sockets::family(IPV4));

    /**
     * @brief Construct from system sockaddr_storage structure.
     * @param addr Socket address storage structure
     *
     * Creates socket address object from existing system address structure.
     * Extracts IP, port, and family information from the structure.
     */
    explicit socket_address(sockaddr_storage& addr);

    /**
     * @brief Custom copy constructor.
     * @param other Socket address to copy from
     *
     * Creates deep copy of socket address including underlying sockaddr structure.
     */
    socket_address(const socket_address& other);

    /**
     * @brief Custom copy assignment operator.
     * @param other Socket address to copy from
     * @return Reference to this object
     *
     * Assigns socket address components and recreates sockaddr structure.
     */
    socket_address& operator=(const socket_address& other);

    // Move operations
    socket_address(socket_address&&) = default;
    socket_address& operator=(socket_address&&) = default;

    /// Default destructor
    ~socket_address() = default;

    /**
     * @brief Get the IP address component (STL-style accessor).
     * @return IP address object
     */
    cppress::sockets::ip_address address() const { return address_; }

    /**
     * @brief Get the port component (STL-style accessor).
     * @return Port object
     */
    cppress::sockets::port port() const { return port_; }

    /**
     * @brief Get the address family component (STL-style accessor).
     * @return Address family object
     */
    cppress::sockets::family family() const { return family_; }

    /**
     * @brief Legacy accessor for IP address (backward compatibility).
     * @deprecated Use address() instead
     */
    [[deprecated("Use address() instead")]]
    ip_address get_ip_address() const {
        return address();
    }

    /**
     * @brief Legacy accessor for port (backward compatibility).
     * @deprecated Use port() instead
     */
    [[deprecated("Use port() instead")]]
    cppress::sockets::port get_port() const {
        return port();
    }

    /**
     * @brief Legacy accessor for family (backward compatibility).
     * @deprecated Use family() instead
     */
    [[deprecated("Use family() instead")]]
    cppress::sockets::family get_family() const {
        return family();
    }

    std::string to_string() const {
        return address_.string() + ":" + std::to_string(port_.value());
    }

    /**
     * @brief Get raw sockaddr pointer for system calls (STL-style accessor).
     * @return Pointer to underlying sockaddr structure
     *
     * Returns pointer suitable for use with socket system calls like
     * bind(), connect(), accept(), etc.
     * Follows STL convention like std::vector::data().
     */
    sockaddr* data() const;

    /**
     * @brief Get size of sockaddr structure (STL-style accessor).
     * @return Size in bytes of the sockaddr structure
     *
     * Returns appropriate size for IPv4 (sockaddr_in) or IPv6 (sockaddr_in6).
     * Follows STL convention like std::vector::size().
     */
    socklen_t size() const;

    /**
     * @brief Legacy accessor for sockaddr pointer (backward compatibility).
     * @deprecated Use data() instead
     */
    [[deprecated("Use data() instead")]]
    sockaddr* get_sock_addr() const {
        return data();
    }

    /**
     * @brief Legacy accessor for sockaddr size (backward compatibility).
     * @deprecated Use size() instead
     */
    [[deprecated("Use size() instead")]]
    socklen_t get_sock_addr_len() const {
        return size();
    }

    /// Allow helper functions to access private members
    friend void handle_ipv4(socket_address* addr, const ip_address& address,
                            const cppress::sockets::port& port_id,
                            const cppress::sockets::family& family_id);
    friend void handle_ipv6(socket_address* addr, const ip_address& address,
                            const cppress::sockets::port& port_id,
                            const cppress::sockets::family& family_id);

    /**
     * @brief Stream insertion operator for output.
     * @param os Output stream
     * @param sa Socket address object to output
     * @return Reference to output stream
     */
    friend std::ostream& operator<<(std::ostream& os, const socket_address& sa) {
        os << "IP Address: " << sa.address() << ", Port: " << sa.port()
           << ", Family: " << sa.family();
        return os;
    }
};

/**
 * @brief Helper function to handle IPv4 address initialization.
 * @param addr Socket address object to initialize
 * @param address IP address component
 * @param port_id Port component
 * @param family_id Address family component
 *
 * Creates and initializes sockaddr_in structure for IPv4 addresses.
 */
void handle_ipv4(socket_address* addr, const ip_address& address,
                 const cppress::sockets::port& port_id, const cppress::sockets::family& family_id);

/**
 * @brief Helper function to handle IPv6 address initialization.
 * @param addr Socket address object to initialize
 * @param address IP address component
 * @param port_id Port component
 * @param family_id Address family component
 *
 * Creates and initializes sockaddr_in6 structure for IPv6 addresses.
 */
void handle_ipv6(socket_address* addr, const ip_address& address,
                 const cppress::sockets::port& port_id, const cppress::sockets::family& family_id);
}  // namespace cppress::sockets