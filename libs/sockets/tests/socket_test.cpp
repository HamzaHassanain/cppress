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
    EXPECT_NO_THROW(client_conn.connect(addr));

    server_thread.join();
    EXPECT_TRUE(connection_accepted);

    cleanup_socket_library();
}

TEST(SocketTest, ConnectOperation) {
    initialize_socket_library();

    cppress::sockets::socket server_sock(family::ipv4(), socket::type::stream);
    socket_address server_addr(ip_address("127.0.0.1"), get_random_free_port(), family::ipv4());
    server_sock.bind(server_addr);
    server_sock.listen();

    std::thread server_thread([&]() {
        std::shared_ptr<connection> conn = server_sock.accept();
        EXPECT_TRUE(conn->is_open());
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    cppress::sockets::socket client_sock(family::ipv4(), socket::type::stream);
    EXPECT_TRUE(client_sock.is_open());

    EXPECT_NO_THROW(client_sock.connect(server_addr));

    server_thread.join();
    cleanup_socket_library();
}

TEST(SocketTest, CloseAndIsOpen) {
    initialize_socket_library();

    cppress::sockets::socket sock(family::ipv4(), socket::type::stream);
    EXPECT_TRUE(sock.is_open());

    if (sock) {
        EXPECT_TRUE(true);
    } else {
        FAIL() << "Socket should be open";
    }

    sock.close();
    EXPECT_FALSE(sock.is_open());

    if (!sock) {
        EXPECT_TRUE(true);
    } else {
        FAIL() << "Socket should be closed";
    }

    cleanup_socket_library();
}

TEST(SocketTest, MultithreadedMultipleSimultaneousConnections) {
    initialize_socket_library();

    cppress::sockets::socket server_sock(family::ipv4(), socket::type::stream);
    socket_address server_addr(ip_address("127.0.0.1"), get_random_free_port(), family::ipv4());
    server_sock.bind(server_addr);
    server_sock.listen();

    const int NUM_CLIENTS = 15;
    std::atomic<int> accepted_connections{0};
    std::atomic<int> successful_connects{0};

    std::thread server_thread([&]() {
        std::vector<std::thread> handler_threads;

        for (int i = 0; i < NUM_CLIENTS; ++i) {
            try {
                std::shared_ptr<connection> conn = server_sock.accept();
                accepted_connections++;

                handler_threads.emplace_back([conn = std::move(conn)]() mutable {
                    try {
                        data_buffer data = conn->read();
                        conn->write(data);  // Echo back
                    } catch (...) {
                    }
                });
            } catch (...) {
                break;
            }
        }

        for (auto& t : handler_threads) {
            if (t.joinable())
                t.join();
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Launch multiple client threads
    std::vector<std::thread> client_threads;
    for (int i = 0; i < NUM_CLIENTS; ++i) {
        client_threads.emplace_back([&server_addr, &successful_connects, i]() {
            try {
                connection conn(server_addr);

                std::string message = "Client " + std::to_string(i);
                conn.write(data_buffer(message));

                data_buffer response = conn.read();
                if (response.to_string() == message) {
                    successful_connects++;
                }
            } catch (...) {
                // Connection failed
            }
        });
    }

    for (auto& t : client_threads) {
        if (t.joinable())
            t.join();
    }

    server_thread.join();

    EXPECT_EQ(accepted_connections.load(), NUM_CLIENTS);
    EXPECT_EQ(successful_connects.load(), NUM_CLIENTS);

    cleanup_socket_library();
}

TEST(SocketTest, MultithreadedSocketOperationsThreadSafety) {
    initialize_socket_library();

    const int NUM_SERVERS = 5;
    std::atomic<int> servers_started{0};
    std::atomic<int> clients_connected{0};
    std::vector<std::thread> server_threads;
    std::vector<std::thread> client_threads;
    std::vector<port> server_ports;

    for (int i = 0; i < NUM_SERVERS; ++i) {
        port p = get_random_free_port();
        server_ports.push_back(p);

        server_threads.emplace_back([p, &servers_started, &clients_connected]() {
            try {
                cppress::sockets::socket server_sock(family::ipv4(), socket::type::stream);
                socket_address addr(ip_address("127.0.0.1"), p, family::ipv4());
                server_sock.bind(addr);
                server_sock.listen();
                servers_started++;

                auto conn = server_sock.accept();
                data_buffer data = conn->read();
                conn->write(data);  // Echo
            } catch (...) {
            }
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    for (int i = 0; i < NUM_SERVERS; ++i) {
        client_threads.emplace_back([&server_ports, i, &clients_connected]() {
            try {
                cppress::sockets::socket client_sock(family::ipv4(), socket::type::stream);
                socket_address addr(ip_address("127.0.0.1"), server_ports[i], family::ipv4());
                auto conn = client_sock.connect(addr);

                std::string message = "Thread " + std::to_string(i);
                conn->write(data_buffer(message));

                data_buffer response = conn->read();
                if (response.to_string() == message) {
                    clients_connected++;
                }
            } catch (...) {
                // Client failed
            }
        });
    }

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
