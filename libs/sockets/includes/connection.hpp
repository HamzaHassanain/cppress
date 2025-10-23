/**
 * @file connection.hpp
 * @brief TCP connection management with STL-style interface.
 *
 * This file provides the connection class for managing established TCP connections
 * in the cppress sockets library. It encapsulates a connected socket with its
 * local and remote endpoints, providing type-safe I/O operations and RAII-based
 * resource management.
 * @section usage Common Usage Patterns
 *
 * **Basic Connection Handling:**
 * @code
 * #include "connection.hpp"
 * using namespace cppress::sockets;
 *
 * // Accept connection from server socket
 * socket server(socket_address(port(8080)), type::stream);
 * server.listen();
 * auto conn = server.accept();
 *
 * // Check connection state
 * if (conn->is_open()) {
 *     // Connection is active
 * }
 *
 * // Use explicit bool conversion
 * if (*conn) {
 *     // Connection is active
 * }
 * @endcode
 *
 * **Sending Data:**
 * @code
 * // Create data to send
 * data_buffer request;
 * request.append("GET / HTTP/1.1\r\n");
 * request.append("Host: example.com\r\n\r\n");
 *
 * // Send data
 * try {
 *     std::size_t bytes_sent = conn->write(request);
 *     std::cout << "Sent " << bytes_sent << " bytes\n";
 * } catch (socket_exception& e) {
 *     std::cerr << "Send failed: " << e.what() << "\n";
 * }
 * @endcode
 *
 * **Receiving Data:**
 * @code
 * // Receive data from connection
 * try {
 *     data_buffer response = conn->read();
 *     std::string content = response.to_string();
 *     std::cout << "Received: " << content << "\n";
 * } catch (const socket_exception& e) {
 *     std::cerr << "Read failed: " << e.what() << "\n";
 * }
 * @endcode
 *
 * **Resource Management:**
 * @code
 * {
 *     auto conn = server.accept();
 *     // Use connection
 *     conn->write(data);
 *     auto response = conn->read();
 *
 *     // Explicit close (optional)
 *     conn->close();
 * } // Automatic cleanup when conn goes out of scope
 * @endcode
 *
 * **Move Semantics:**
 * @code
 * // Connections are move-only
 * auto conn1 = server.accept();
 * auto conn2 = std::move(conn1);  // Transfer ownership
 * // conn1 is now invalid, only conn2 can be used
 *
 * // Store in containers
 * std::vector<std::shared_ptr<connection>> connections;
 * connections.push_back(server.accept());
 * @endcode
 *
 * @section integration Integration with Sockets Library
 * The connection class integrates with other cppress::sockets components:
 * - Created by socket::accept() for TCP servers
 * - Uses data_buffer for all I/O operations
 * - Wraps file_descriptor for platform-specific handles
 * - Contains socket_address for endpoint information
 * - Throws socket_exception on errors
 *
 * @section exceptions Exception Safety
 * Methods that can fail throw socket_exception with specific error types:
 * - "ProtocolMismatch": Called on non-TCP socket
 * - "SocketWrite": Write operation failed
 * - "PartialWrite": Not all data was sent
 * - "SocketRead": Read operation failed
 *
 * @section threading Thread Safety
 * - Connection objects are NOT thread-safe
 * - Each connection should be used by a single thread
 * - Or protected by external synchronization
 * - Move operations must be externally synchronized
 *
 * @author Hamza Mohammed Hassanain
 * @version 1.0
 */

#pragma once

#include "data_buffer.hpp"
#include "file_descriptor.hpp"
#include "socket_address.hpp"
#include "utilities.hpp"

namespace cppress::sockets {
/**
 * @brief Represents a connection to a remote socket.
 * This class provides an interface for sending and receiving data
 * over the established TCP connection.
 */
class connection {
private:
    /// File descriptor for the socket
    file_descriptor fd;

    /// Address of the local socket
    socket_address local_addr;

    /// Address of the remote socket
    socket_address remote_addr;

    /// flag to indicate if the connection is open
    bool open_ = true;

public:
    /**
     * @brief Construct a new connection object
     *
     */
    connection() = default;
    /**
     * @brief Construct a new connection object
     *
     * @param fd File descriptor representing the socket
     * @param local_addr Address of the local socket
     * @param remote_addr Address of the remote socket
     *
     * @throw socket_exception if the file descriptor is invalid
     */
    connection(file_descriptor fd, const socket_address& local_addr,
               const socket_address& remote_addr);

    /**
     * @brief Construct a new connection object and connect to remote address (server)
     * @param remote_addr Address of the remote socket to connect to
     * @throw socket_exception if the connection fails
     */
    connection(const socket_address& remote_addr);

    ~connection();

    // Deleted copy constructor and assignment operator
    connection(const connection&) = delete;
    connection& operator=(const connection&) = delete;

    // Move constructor and assignment operator
    connection(connection&& other) noexcept {
        fd = std::move(other.fd);
        local_addr = other.local_addr;
        remote_addr = other.remote_addr;
        open_ = other.open_;
        other.open_ = false;

        other.fd.invalidate();
    }
    connection& operator=(connection&& other) noexcept {
        if (this != &other) {
            fd = std::move(other.fd);
            local_addr = other.local_addr;
            remote_addr = other.remote_addr;
            open_ = other.open_;
            other.open_ = false;

            other.fd.invalidate();
        }
        return *this;
    }

    /**
     * @brief Get the file descriptor raw value (STL-style accessor).
     * @return the value of the file descriptor
     *
     * Returns the underlying platform-specific file descriptor.
     * Follows STL convention like std::thread::native_handle().
     */
    int native_handle() const { return fd.native_handle(); }

    /**
     * @brief Legacy accessor for file descriptor (backward compatibility).
     * @deprecated Use native_handle() instead
     * @return the value of the file descriptor
     */
    [[deprecated("Use native_handle() instead")]]
    int get_fd() const {
        return native_handle();
    }

    /**
     * @brief Send data on established connection (STL-style).
     * @param data Data buffer to send
     * @throws socket_exception with type "ProtocolMismatch" if called on non-TCP socket
     * @throws socket_exception with type "SocketWrite" if write operation fails
     * @throws socket_exception with type "PartialWrite" if not all data was sent
     * @return Number of sent bytes
     *
     * Follows standard socket API naming conventions.
     */
    std::size_t write(const data_buffer& data);

    /**
     * @brief Legacy method for sending data (backward compatibility).
     * @deprecated Use write() instead
     */
    [[deprecated("Use write() instead")]]
    std::size_t send(const data_buffer& data);

    /**
     * @brief Receive data from established connection (STL-style).
     * @return Buffer containing received data
     * @throws socket_exception with type "ProtocolMismatch" if called on non-TCP socket
     * @throws socket_exception with type "SocketRead" if read operation fails
     *
     * Follows standard socket API naming conventions.
     */
    data_buffer read();

    /**
     * @brief Legacy method for receiving data (backward compatibility).
     * @deprecated Use read() instead
     */
    [[deprecated("Use read() instead")]]
    data_buffer receive();

    /**
     * @brief Close the connection (STL-style).
     *
     * This function will close the socket and release any resources
     * associated with the connection.
     * Follows STL convention like std::fstream::close().
     */
    void close();

    /**
     * @brief Check if the connection is open (STL-style accessor).
     * @return true if the connection is open, false otherwise
     *
     * Follows STL convention like std::fstream::is_open().
     */
    bool is_open() const;

    /**
     * @brief Explicit bool conversion operator.
     * @return true if connection is open, false otherwise
     *
     * Allows using connection in boolean contexts:
     * @code
     * connection conn(...);
     * if (conn) { // Checks if connection is open
     *     // Use connection
     * }
     * @endcode
     */
    explicit operator bool() const { return is_open(); }

    /**
     * @brief Legacy method for checking connection state (backward compatibility).
     * @deprecated Use is_open() instead
     * @return true if the connection is open, false otherwise
     */
    [[deprecated("Use is_open() instead")]]
    bool is_connection_open() const;

    /**
     * @brief Get the remote endpoint address (STL-style accessor).
     * @return socket_address of the remote peer
     *
     * Follows STL naming like networking TS and Boost.Asio.
     */
    socket_address remote_endpoint() const { return remote_addr; }

    /**
     * @brief Get the local endpoint address (STL-style accessor).
     * @return socket_address of the local endpoint
     *
     * Follows STL naming like networking TS and Boost.Asio.
     */
    socket_address local_endpoint() const { return local_addr; }

    /**
     * @brief Legacy accessor for remote address (backward compatibility).
     * @deprecated Use remote_endpoint() instead
     * @return socket_address
     */
    [[deprecated("Use remote_endpoint() instead")]]
    socket_address get_remote_address() const {
        return remote_endpoint();
    }

    /**
     * @brief Legacy accessor for local address (backward compatibility).
     * @deprecated Use local_endpoint() instead
     * @return socket_address
     */
    [[deprecated("Use local_endpoint() instead")]]
    socket_address get_local_address() const {
        return local_endpoint();
    }

    /**
     * @brief Connect to a remote address (server).
     * @param remote_addr The address of the remote server
     */
    void connect(const socket_address& remote_addr);
};
}  // namespace cppress::sockets