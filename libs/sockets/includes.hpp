/**
 * @file includes.hpp
 * @brief Main header file for the cppress sockets library.
 *
 * This library provides a modern C++ socket programming framework with an STL-like
 * interface and RAII principles. It supports cross-platform TCP/UDP networking with
 * familiar C++ Standard Library patterns and naming conventions.
 *
 * @section features Features
 * - Cross-platform socket support (Windows, Linux, Unix)
 * - STL-like interfaces with standard naming conventions
 * - RAII-based resource management for automatic cleanup
 * - Type-safe socket address and endpoint management
 * - Efficient binary data handling with data_buffer
 * - TCP server implementations (threaded and epoll-based)
 * - Move semantics for efficient resource transfer
 * - Comprehensive error handling with custom exceptions
 * - Doxygen-documented API
 *
 * @section types Core Types
 * - socket: Main socket class for TCP/UDP operations with close(), is_open()
 * - connection: Represents an established TCP connection with write(), read()
 * - socket_address: Complete socket address with IP, port, and family
 * - data_buffer: Dynamic buffer for binary data with STL-like interface
 * - tcp_server: Multi-threaded TCP server
 * - epoll_server: High-performance epoll-based server (Linux)
 *
 * @section components Basic Components
 * - file_descriptor: Cross-platform file descriptor wrapper with native_handle()
 * - ip_address: Type-safe IP address with string() accessor
 * - port: Validated port number (1024-65535) with value() accessor
 * - family: Address family wrapper (IPv4/IPv6) with value() accessor
 *
 * @section namespaces Namespaces
 * - cppress::sockets: Main namespace containing all socket types and utilities
 *
 * @section example Basic Usage Examples
 * @code
 * #include "includes.hpp"
 * using namespace cppress::sockets;
 *
 * // Create a TCP client
 * socket client_sock(family::ipv4(), socket::type::stream);
 * socket_address server_addr(ip_address("127.0.0.1"), port(8080), family::ipv4());
 * client_sock.connect(server_addr);
 *
 * // Send data
 * data_buffer request("GET / HTTP/1.1\r\n\r\n");
 * client_sock.send(request);
 *
 * // Receive response
 * auto response = client_sock.receive();
 * std::cout << "Received: " << response.to_string() << std::endl;
 *
 * // Socket automatically closes (RAII)
 * if (client_sock.is_open()) {
 *     client_sock.close();
 * }
 *
 * // Create a TCP server
 * tcp_server server(port(8080), family::ipv4());
 * server.on_connection([](connection& conn) {
 *     // Handle client connection
 *     auto request = conn.read();
 *     std::cout << "Client sent: " << request.to_string() << std::endl;
 *
 *     // Send response
 *     data_buffer response("Hello from server!");
 *     conn.write(response);
 * });
 * server.start();
 *
 * // Working with connections
 * socket server_sock(family::ipv4(), socket::type::stream);
 * server_sock.bind(socket_address(ip_address("0.0.0.0"), port(8080), family::ipv4()));
 * server_sock.listen();
 *
 * connection client = server_sock.accept();
 * if (client.is_open()) {
 *     // Get endpoint information (Networking TS style)
 *     auto remote = client.remote_endpoint();
 *     auto local = client.local_endpoint();
 *
 *     std::cout << "Client: " << remote.address().string()
 *               << ":" << remote.port().value() << std::endl;
 *
 *     // STL-like I/O operations
 *     auto data = client.read();
 *     client.write(data);  // Echo back
 *
 *     // Explicit close or automatic via RAII
 *     client.close();
 * }
 *
 * // Data buffer operations (STL-style)
 * data_buffer buf;
 * buf.append("Hello");
 * buf.append(" World");
 *
 * if (!buf.empty()) {
 *     std::cout << "Size: " << buf.size() << std::endl;
 *     std::cout << "Data: " << buf.to_string() << std::endl;
 *     const char* raw = buf.data();  // Raw pointer access
 * }
 * buf.clear();
 *
 * // Type-safe address components
 * ip_address addr("192.168.1.1");
 * std::string addr_str = addr.string();  // STL-style accessor
 *
 * port p(8080);
 * int port_num = p.value();  // STL-style accessor
 *
 * family fam = family::ipv4();
 * int af = fam.value();  // Get AF_INET
 *
 * // Complete socket address
 * socket_address endpoint(ip_address("10.0.0.1"), port(3000), family::ipv4());
 * auto ip = endpoint.address();    // Get ip_address component
 * auto pt = endpoint.port();       // Get port component
 * auto fm = endpoint.family();     // Get family component
 * auto* raw_addr = endpoint.data(); // Get raw sockaddr* (STL-style)
 * auto addr_len = endpoint.size();  // Get sockaddr length (STL-style)
 * @endcode
 *
 * @section platform Platform Support
 * The library automatically detects the platform and uses appropriate system calls:
 * - Windows: Winsock2 API
 * - Linux/Unix: BSD sockets API
 * - Linux: Optional epoll for high-performance servers
 *
 * Platform detection is handled automatically via preprocessor directives.
 *
 * @section design Design Principles
 * - **RAII**: All resources (sockets, file descriptors) automatically cleaned up
 * - **Move-only semantics**: Sockets and connections cannot be copied, only moved
 * - **STL naming**: Methods follow standard library conventions (close(), is_open(),
 *   data(), size(), value(), string(), native_handle())
 * - **Type safety**: Strong typing for IP addresses, ports, and address families
 * - **Networking TS compatibility**: endpoint(), remote_endpoint(), local_endpoint()
 * - **Explicit constructors**: Prevents implicit conversions for type safety
 * - **Backward compatibility**: Deprecated methods available with [[deprecated]] attributes
 *
 * @section migration Migration from Legacy API
 * Old methods are deprecated but still available:
 * - get() → value() or string() or native_handle()
 * - get_fd() → native_handle()
 * - disconnect() → close()
 * - is_connected() → is_open()
 * - send() → write()
 * - receive() → read()
 * - get_remote_address() → remote_endpoint()
 * - get_local_address() → local_endpoint()
 *
 * @author Hamza Moahmmed Hassanain
 * @version 2.0
 */

#pragma once

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#ifndef SOCKET_PLATFORM_WINDOWS
#define SOCKET_PLATFORM_WINDOWS
#endif
#else
#ifndef SOCKET_PLATFORM_UNIX
#define SOCKET_PLATFORM_UNIX
#endif
#endif

#include "includes/connection.hpp"
#include "includes/data_buffer.hpp"
#include "includes/epoll_server.hpp"
#include "includes/exceptions.hpp"
#include "includes/family.hpp"
#include "includes/file_descriptor.hpp"
#include "includes/ip_address.hpp"
#include "includes/port.hpp"
#include "includes/socket.hpp"
#include "includes/socket_address.hpp"
#include "includes/tcp_server.hpp"
#include "includes/utilities.hpp"