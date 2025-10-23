/**
 * @file test_socket.cpp
 * @brief Unit tests for the socket class
 */

#include "includes/socket.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include "includes/connection.hpp"
#include "includes/data_buffer.hpp"
#include "includes/socket_address.hpp"
#include "includes/utilities.hpp"

using namespace cppress::sockets;

/**
 * @test Test socket bind, listen, and accept operations
 * Verifies basic server socket functionality
 */
TEST(SocketTest, BindListenAccept) {
    initialize_socket_library();

    cppress::sockets::socket server_sock(family::ipv4(), socket::type::stream);
    EXPECT_TRUE(server_sock.is_open());

    port server_port = get_random_free_port();
    socket_address addr(ip_address("127.0.0.1"), server_port, family::ipv4());
    EXPECT_NO_THROW(server_sock.bind(addr));

    EXPECT_NO_THROW(server_sock.listen());

    std::atomic<bool> connection_accepted{false};
    std::thread server_thread([&]() {
        std::shared_ptr<connection> client_conn = server_sock.accept();
        EXPECT_TRUE(client_conn->is_open());
        connection_accepted = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    connection client_conn;
    EXPECT_NO_THROW(client_conn.connect());

    server_thread.join();
    EXPECT_TRUE(connection_accepted);

    cleanup_socket_library();
}

/**
 * @test Test socket connect operation
 * Verifies client socket can connect to server
 */
TEST(SocketTest, ConnectOperation) {
    initialize_socket_library();

    // Create server
    cppress::sockets::socket server_sock(family::ipv4(), socket::type::stream);
    socket_address server_addr(ip_address("127.0.0.1"), get_random_free_port(), family::ipv4());
    server_sock.bind(server_addr);
    server_sock.listen();

    std::thread server_thread([&]() {
        std::shared_ptr<connection> conn = server_sock.accept();
        EXPECT_TRUE(conn->is_open());
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Create client and connect
    cppress::sockets::socket client_sock(family::ipv4(), socket::type::stream);
    EXPECT_TRUE(client_sock.is_open());

    EXPECT_NO_THROW(client_sock.connect(server_addr));

    server_thread.join();
    cleanup_socket_library();
}

/**
 * @test Test socket close and is_open operations
 * Verifies socket state management
 */
TEST(SocketTest, CloseAndIsOpen) {
    initialize_socket_library();

    // Create socket
    cppress::sockets::socket sock(family::ipv4(), socket::type::stream);
    EXPECT_TRUE(sock.is_open());

    // Test explicit operator bool
    if (sock) {
        EXPECT_TRUE(true);
    } else {
        FAIL() << "Socket should be open";
    }

    // Close socket
    sock.close();
    EXPECT_FALSE(sock.is_open());

    // Test operator bool after close
    if (!sock) {
        EXPECT_TRUE(true);
    } else {
        FAIL() << "Socket should be closed";
    }

    cleanup_socket_library();
}

/**
 * @test MULTITHREADING: Test multiple clients connecting simultaneously
 * Verifies thread safety for concurrent connections
 */
TEST(SocketTest, MultithreadedMultipleSimultaneousConnections) {
    initialize_socket_library();

    // Create server socket
    cppress::sockets::socket server_sock(family::ipv4(), socket::type::stream);
    socket_address server_addr(ip_address("127.0.0.1"), get_random_free_port(), family::ipv4());
    server_sock.bind(server_addr);
    server_sock.listen();

    const int NUM_CLIENTS = 15;
    std::atomic<int> accepted_connections{0};
    std::atomic<int> successful_connects{0};

    // Server thread accepting multiple connections
    std::thread server_thread([&]() {
        std::vector<std::thread> handler_threads;

        for (int i = 0; i < NUM_CLIENTS; ++i) {
            try {
                std::shared_ptr<connection> conn = server_sock.accept();
                accepted_connections++;

                // Handle connection in separate thread
                handler_threads.emplace_back([conn = std::move(conn)]() mutable {
                    try {
                        data_buffer data = conn->read();
                        conn->write(data);  // Echo back
                    } catch (...) {
                        // Handler failed
                    }
                });
            } catch (...) {
                // Accept failed
                break;
            }
        }

        for (auto& t : handler_threads) {
            if (t.joinable())
                t.join();
        }
    });

    // Give server time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Launch multiple client threads
    std::vector<std::thread> client_threads;
    for (int i = 0; i < NUM_CLIENTS; ++i) {
        client_threads.emplace_back([&server_addr, &successful_connects, i]() {
            try {
                connection conn(server_addr);

                // Send and receive data
                std::string message = "Client " + std::to_string(i);
                conn.send(data_buffer(message));

                data_buffer response = conn.receive();
                if (response.to_string() == message) {
                    successful_connects++;
                }
            } catch (...) {
                // Connection failed
            }
        });
    }

    // Wait for all clients
    for (auto& t : client_threads) {
        if (t.joinable())
            t.join();
    }

    server_thread.join();

    // Verify all connections were successful
    EXPECT_EQ(accepted_connections.load(), NUM_CLIENTS);
    EXPECT_EQ(successful_connects.load(), NUM_CLIENTS);

    cleanup_socket_library();
}

/**
 * @test MULTITHREADING: Test socket thread safety with concurrent operations
 * Verifies that sockets can be safely used across threads
 */
TEST(SocketTest, MultithreadedSocketOperationsThreadSafety) {
    initialize_socket_library();

    const int NUM_SERVERS = 5;
    std::atomic<int> servers_started{0};
    std::atomic<int> clients_connected{0};
    std::vector<std::thread> server_threads;
    std::vector<std::thread> client_threads;
    std::vector<port> server_ports;

    // Create multiple servers on different ports
    for (int i = 0; i < NUM_SERVERS; ++i) {
        port p = get_random_free_port();
        server_ports.push_back(p);

        server_threads.emplace_back([p, &servers_started, &clients_connected]() {
            try {
                socket server_sock(family::ipv4(), socket::type::stream);
                socket_address addr(ip_address("127.0.0.1"), p, family::ipv4());
                server_sock.bind(addr);
                server_sock.listen();
                servers_started++;

                // Accept one connection
                connection conn = server_sock.accept();
                data_buffer data = conn.read();
                conn.write(data);  // Echo
            } catch (...) {
                // Server failed
            }
        });
    }

    // Wait for all servers to start
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Create clients to connect to each server
    for (int i = 0; i < NUM_SERVERS; ++i) {
        client_threads.emplace_back([&server_ports, i, &clients_connected]() {
            try {
                socket client_sock(family::ipv4(), socket::type::stream);
                socket_address addr(ip_address("127.0.0.1"), server_ports[i], family::ipv4());
                client_sock.connect(addr);

                std::string message = "Thread " + std::to_string(i);
                client_sock.send(data_buffer(message));

                data_buffer response = client_sock.receive();
                if (response.to_string() == message) {
                    clients_connected++;
                }
            } catch (...) {
                // Client failed
            }
        });
    }

    // Wait for completion
    for (auto& t : server_threads) {
        if (t.joinable())
            t.join();
    }
    for (auto& t : client_threads) {
        if (t.joinable())
            t.join();
    }

    EXPECT_EQ(servers_started.load(), NUM_SERVERS);
    EXPECT_EQ(clients_connected.load(), NUM_SERVERS);

    cleanup_socket_library();
}
