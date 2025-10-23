/**
 * @file socket.hpp
 * @brief Cross-platform TCP/UDP socket implementation with STL-style interface.
 *
 * This file provides the core socket class for network programming in the cppress
 * sockets library. It offers a unified, high-level interface for both TCP and UDP
 * socket_types with automatic resource management and cross-platform compatibility.
 *
 * @section features Features
 * - STL-style interface (is_open(), close(), native_handle())
 * - Support for both TCP (connection-oriented) and UDP (connectionless)
 * - Cross-platform compatibility (Windows/Linux/Unix)
 * - RAII-based automatic resource cleanup
 * - Move-only semantics for unique ownership
 * - Comprehensive socket options (SO_REUSEADDR, non-blocking, keep-alive)
 * - Type-safe socket_type selection
 * - Backward-compatible deprecated methods for migration
 *
 * @section socket_types Protocol Support
 *
 * **TCP (Transmission Control Protocol):**
 * - Connection-oriented, reliable, ordered delivery
 * - Methods: bind(), listen(), accept(), connect()
 * - Returns connection objects for established connections
 * - Suitable for HTTP servers, chat applications, file transfers
 *
 * **UDP (User Datagram Protocol):**
 * - Connectionless, unreliable, message-oriented
 * - Methods: bind(), send_to(), receive()
 * - No connection establishment required
 * - Suitable for DNS, streaming media, gaming
 *
 * @section usage Common Usage Patterns
 *
 * **TCP Server:**
 * @code
 * #include "socket.hpp"
 * using namespace cppress::sockets;
 *
 * // Create and configure server socket
 * socket_address addr(port(8080), ip_address("0.0.0.0"));
 * socket server(addr, type::stream);
 *
 * // Set socket options
 * server.set_reuse_address(true);
 * server.set_non_blocking(false);
 *
 * // Start listening
 * server.listen(128);  // backlog of 128
 *
 * // Accept connections
 * while (server.is_open()) {
 *     auto client = server.accept();
 *     // Handle client connection
 *     auto request = client->read();
 *     client->write(response);
 * }
 * @endcode
 *
 * **TCP Client:**
 * @code
 * // Create client socket
 * socket client(type::stream);
 *
 * // Connect to server
 * socket_address server_addr(port(8080), ip_address("127.0.0.1"));
 * client.connect(server_addr);
 *
 * if (client.is_open()) {
 *     // Send request
 *     data_buffer request("GET / HTTP/1.1\r\n\r\n");
 *     client.write(request);
 *
 *     // Receive response
 *     auto response = client.read();
 * }
 * @endcode
 *
 * **UDP Server:**
 * @code
 * // Create UDP socket
 * socket_address addr(port(5353), ip_address("0.0.0.0"));
 * socket udp_server(addr, type::datagram);
 *
 * // Receive datagrams from any client
 * while (udp_server.is_open()) {
 *     socket_address client_addr;
 *     data_buffer message = udp_server.receive(client_addr);
 *
 *     // Process message and respond
 *     data_buffer response = process(message);
 *     udp_server.send_to(client_addr, response);
 * }
 * @endcode
 *
 * **UDP Client:**
 * @code
 * // Create UDP socket (no bind needed for client)
 * socket udp_client(type::datagram);
 *
 * // Send datagram
 * socket_address server(port(5353), ip_address("8.8.8.8"));
 * data_buffer query = create_dns_query();
 * udp_client.send_to(server, query);
 *
 * // Receive response
 * socket_address responder;
 * data_buffer response = udp_client.receive(responder);
 * @endcode
 *
 * **Socket Options:**
 * @code
 * socket s(addr, type::stream);
 *
 * // Reuse address (avoid "Address already in use" errors)
 * s.set_reuse_address(true);
 *
 * // Non-blocking I/O
 * s.set_non_blocking(true);
 *
 * // TCP keep-alive (TCP only)
 * s.set_option(SOL_SOCKET, SO_KEEPALIVE, 1);
 *
 * // Close-on-exec flag
 * s.set_close_on_exec(true);
 *
 * // Custom socket options
 * s.set_option(IPPROTO_TCP, TCP_NODELAY, 1);
 * @endcode
 *
 * **Resource Management:**
 * @code
 * {
 *     socket s(addr, type::stream);
 *     s.listen();
 *     // Use socket
 *
 *     // Explicit close (optional)
 *     s.close();
 * } // Automatic cleanup when s goes out of scope
 *
 * // Move semantics for ownership transfer
 * socket s1(addr, type::stream);
 * socket s2 = std::move(s1);  // Transfer ownership
 * // s1 is now invalid, only s2 owns the socket
 * @endcode
 *
 * **Boolean Context:**
 * @code
 * socket s(addr, type::stream);
 * if (s) {  // Checks if socket is open
 *     // Socket is valid and open
 * }
 * @endcode
 *
 * @section integration Integration with Sockets Library
 * The socket class integrates with other cppress::sockets components:
 * - Uses socket_address for bind/connect/accept operations
 * - Returns connection objects from accept()
 * - Uses data_buffer for UDP send/receive
 * - Wraps file_descriptor for platform-specific handles
 * - Throws socket_exception on errors
 *
 * @section exceptions Exception Safety
 * Methods throw socket_exception with specific error types:
 * - "SocketCreation": Failed to create socket
 * - "SocketBinding": Failed to bind to address
 * - "SocketListening": Failed to enter listening state
 * - "SocketAcceptance": Failed to accept connection
 * - "SocketOption": Failed to set socket option
 * - "ProtocolMismatch": Method called on wrong socket_type type
 * - "SocketSend"/"SocketReceive": UDP I/O failures
 * - "PartialSend": Not all data sent in UDP operation
 *
 * @section platform Platform-Specific Notes
 *
 * **Windows:**
 * - Uses Winsock2 (requires WSAStartup initialization)
 * - Socket handles are SOCKET type (unsigned int)
 * - Some options like TCP_QUICKACK not available
 * - Uses ioctlsocket() for non-blocking mode
 *
 * **Linux/Unix:**
 * - Socket handles are int file descriptors
 * - Uses fcntl() for non-blocking mode
 * - Full support for all POSIX socket options
 * - Supports TCP_QUICKACK and other Linux-specific options
 *
 * @section threading Thread Safety
 * - Socket objects are NOT thread-safe
 * - Each socket should be used by a single thread
 * - Or protected by external synchronization
 * - Move operations must be externally synchronized
 *
 * @section performance Performance Characteristics
 * - Socket creation: System call overhead
 * - Bind/listen/connect: System call overhead
 * - Accept: Blocking or non-blocking based on configuration
 * - UDP send/receive: Direct system calls
 * - Move operations: O(1) constant time
 * - No dynamic allocation during I/O operations
 *
 * @author Hamza Mohammed Hassanain
 * @version 1.0
 */

#pragma once

#include <string>

// Platform-specific includes
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netinet/in.h>
#endif
#include <atomic>

#include "connection.hpp"
#include "data_buffer.hpp"
#include "exceptions.hpp"
#include "file_descriptor.hpp"
#include "socket_address.hpp"
#include "utilities.hpp"

namespace cppress::sockets {
/**
 * @brief Cross-platform socket wrapper for TCP and UDP network operations.
 *
 * This class provides a high-level, interface for socket programming.
 * It abstracts system-level socket operations and handles resource management
 * automatically. Supports both TCP and UDP socket_types with separate method sets
 * for connection-oriented and connectionless operations.
 *
 * Platform Support:
 * - Linux/Unix: Full support for all socket options
 * - Windows: Full support except TCP_QUICKACK (Windows-specific alternatives may vary)
 *
 * The class implements move-only semantics to ensure unique ownership of
 * socket resources and prevent accidental duplication or resource leaks.
 *
 * @note Uses explicit constructors to prevent implicit conversions
 * @note Move-only design prevents copying socket resources
 * @note Cross-platform compatibility with conditional compilation
 * @note Automatically handles socket cleanup in destructor
 */
class socket {
public:
    /**
     * @brief Socket types
     *
     * This struct defines the various socket types supported by the library.
     * These types correspond to the socket types defined in the POSIX standard.
     * The types include:
     * - stream: for TCP sockets (SOCK_STREAM)
     * - datagram: for UDP sockets (SOCK_DGRAM)
     */
    enum class type { stream = SOCK_STREAM, datagram = SOCK_DGRAM };

private:
    /// Socket address (IP, port, family)
    socket_address addr;
    /// Platform-specific file descriptor wrapper
    file_descriptor fd;

    /// Protocol type (TCP or UDP)
    socket::type socket_type;

    /// flag to indicate if the socket is open
    bool open_{true};

public:
    /// Default constructor deleted - sockets must be explicitly configured
    socket() = delete;

    /**
     * @brief Create and bind socket to address.
     * @param socket_type Network socket_type (TCP/UDP)
     * @throws socket_exception with type "SocketCreation" if socket creation fails
     */
    explicit socket(const type& socket_type);

    /**
     * @brief Create and bind socket to address.
     * @param addr Socket address to bind to
     * @param socket_type Network socket_type (TCP/UDP)
     * @note Automatically binds socket to the specified address.
     * @throws socket_exception with type "SocketCreation" if socket creation fails
     * @throws socket_exception with type "SocketBinding" if binding fails
     */
    explicit socket(const socket_address& addr, const type& socket_type);

    /**
     * @brief Create and bind socket to address.
     * @param addr Socket address to bind to
     * @param socket_type Network socket_type (TCP/UDP)
     * @note Automatically binds socket to the specified address.
     * @throws socket_exception with type "SocketCreation" if socket creation fails
     * @throws socket_exception with type "SocketBinding" if binding fails
     */
    explicit socket(const family& fam, const type& socket_type);

    // Copy operations - DELETED for resource safety
    socket(const socket&) = delete;
    socket& operator=(const socket&) = delete;

    /**
     * @brief Move constructor.
     * @param other Socket to move from
     *
     * Transfers ownership of socket resources. Source socket becomes invalid.
     */
    socket(socket&& other)
        : addr(std::move(other.addr)), fd(std::move(other.fd)), socket_type(other.socket_type) {}

    /**
     * @brief Move assignment operator.
     * @param other Socket to move from
     * @return Reference to this socket
     *
     * Transfers ownership of socket resources from another socket.
     */
    socket& operator=(socket&& other) {
        if (this != &other) {
            addr = std::move(other.addr);
            fd = std::move(other.fd);
            socket_type = other.socket_type;
        }
        return *this;
    }

    /**
     * @brief Binds socket to the specified address.
     * @param addr Address to bind to
     * @throws socket_exception with type "SocketBinding" if bind operation fails
     */
    void bind(const socket_address& addr);

    /**
     * @brief Sets SO_REUSEADDR socket option.
     * @param reuse Whether to enable address reuse
     * @throws socket_exception with type "SocketOption" if setsockopt fails
     */
    void set_reuse_address(bool reuse);

    /**
     * @brief Sets socket to non-blocking or blocking mode.
     * @param enable Whether to enable non-blocking mode
     * @throws socket_exception with type "SocketOption" if operation fails
     *
     * Cross-platform implementation:
     * - Unix/Linux: Uses fcntl() with O_NONBLOCK flag
     * - Windows: Uses ioctlsocket() with FIONBIO
     *
     * Non-blocking sockets return immediately from I/O operations even if no data
     * is available, preventing the calling thread from being blocked.
     * Essential for implementing asynchronous I/O and event-driven servers.
     */
    void set_non_blocking(bool enable);

    /**
     * @brief Sets SO_KEEPALIVE socket option to enable TCP keep-alive probes.
     * @param enable Whether to enable keep-alive
     * @throws socket_exception with type "ProtocolMismatch" if called on non-TCP socket
     * @throws socket_exception with type "SocketOption" if setsockopt fails
     *
     * When enabled, TCP automatically sends keep-alive packets to detect
     * dead connections and clean up resources for broken connections.
     * Only applicable to TCP sockets.
     */

    /**
     * @brief Set the close on exec object
     *  that prevents the file descriptor from being inherited by child processes.
     * @param enable Whether to enable close-on-exec
     */

    void set_close_on_exec(bool enable);

    /**
     * @brief Start listening for connections (TCP only).
     * @param backlog Maximum number of pending connections
     * @throws socket_exception with type "ProtocolMismatch" if called on non-TCP socket
     * @throws socket_exception with type "SocketListening" if listen operation fails
     */
    void listen(int backlog = SOMAXCONN);

    /**
     * @brief Accept incoming connection (TCP only).
     * @param NON_BLOCKING Whether to use non-blocking accept for clients (default: false)
     * @return Shared pointer to new socket for the accepted connection
     * @throws socket_exception with type "ProtocolMismatch" if called on non-TCP socket
     * @throws socket_exception with type "SocketAcceptance" if accept operation fails
     */
    std::shared_ptr<connection> accept(bool NON_BLOCKING = false);

    /**
     * @brief Receive data from any client (UDP only).
     * @param client_addr Will be filled with sender's address
     * @return Buffer containing received data
     * @throws socket_exception with type "ProtocolMismatch" if called on non-UDP socket
     * @throws socket_exception with type "SocketReceive" if receive operation fails
     */
    data_buffer receive(socket_address& client_addr);

    /**
     * @brief Send data to specific address (UDP only).
     * @param addr Destination address
     * @param data Data to send
     * @throws socket_exception with type "ProtocolMismatch" if called on non-UDP socket
     * @throws socket_exception with type "SocketSend" if send operation fails
     * @throws socket_exception with type "PartialSend" if not all data was sent
     */
    void send_to(const socket_address& addr, const data_buffer& data);

    /**
     * @brief Get remote endpoint address.
     * @return Socket address of remote endpoint
     */
    socket_address get_bound_address() const;

    /**
     * @brief Get raw file descriptor value (STL-style accessor).
     * @return Integer file descriptor value
     *
     * Returns the underlying platform-specific socket handle or file descriptor.
     * Follows STL convention like std::thread::native_handle().
     */
    int native_handle() const;

    /**
     * @brief Legacy accessor for file descriptor (backward compatibility).
     * @deprecated Use native_handle() instead
     * @return Integer file descriptor value
     */
    [[deprecated("Use native_handle() instead")]]
    int get_fd() const;

    /**
     * @brief Close the socket connection (STL-style).
     *
     * Safely closes the socket and releases system resources.
     * Socket becomes unusable after this call.
     * Follows STL convention like std::fstream::close().
     * @note This method does not throw exceptions - errors are logged to stderr
     */
    void close();

    /**
     * @brief Legacy method for closing socket (backward compatibility).
     * @deprecated Use close() instead
     */
    [[deprecated("Use close() instead")]]
    void disconnect();

    /**
     * @brief Check if socket is open (STL-style accessor).
     * @return true if socket is open and valid, false otherwise
     *
     * Follows STL convention like std::fstream::is_open().
     */
    bool is_open() const;

    /**
     * @brief Explicit bool conversion operator.
     * @return true if socket is open and valid, false otherwise
     *
     * Allows using socket in boolean contexts:
     * @code
     * socket s(...);
     * if (s) { // Checks if socket is open
     *     // Use socket
     * }
     * @endcode
     */
    explicit operator bool() const { return is_open(); }

    /**
     * @brief Legacy method for checking connection (backward compatibility).
     * @deprecated Use is_open() instead
     * @return true if socket has valid connection, false otherwise
     */
    [[deprecated("Use is_open() instead")]]
    bool is_connected() const;

    /**
     * @brief Less-than operator for container ordering.
     * @param other Socket to compare with
     * @return true if this socket's file descriptor is less than other's
     */

    /**
     * @brief Set custom options
     * @param level The level at which the option is defined (e.g., SOL_SOCKET)
     * @param optname The name of the option to set
     * @param optval The value to set the option to
     */
    void set_option(int level, int optname, int optval);

    bool operator<(const socket& other) const { return fd < other.fd; }

    /// Destructor - automatically handles resource cleanup
    ~socket();
};
}  // namespace cppress::sockets