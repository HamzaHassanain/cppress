/**
 * @file test_std::shared_ptr<connection>.cpp
 * @brief Unit tests for the std::shared_ptr<connection> class
 */

#include "includes/connection.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>

#include "includes/data_buffer.hpp"
#include "includes/socket.hpp"
#include "includes/socket_address.hpp"
#include "includes/utilities.hpp"

using namespace cppress::sockets;

TEST(ConnectionTest, BasicWriteAndRead) {
    initialize_socket_library();

    // Create server socket
    cppress::sockets::socket server_sock(family::ipv4(), cppress::sockets::socket::type::stream);
    socket_address server_addr(ip_address("127.0.0.1"), get_random_free_port(), family::ipv4());
    server_sock.bind(server_addr);
    server_sock.listen();

    std::atomic<bool> server_ready{false};
    data_buffer received_data;
    std::thread server_thread([&]() {
        std::shared_ptr<connection> client_conn = server_sock.accept();
        server_ready = true;

        // Read data from client
        received_data = client_conn->read();

        // Echo back
        client_conn->write(received_data);
    });

    // Give server time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    connection client_conn(server_addr);

    // Send data
    data_buffer send_data("Hello from client!");
    size_t bytes_sent = client_conn.write(send_data);
    EXPECT_EQ(bytes_sent, send_data.size());

    // Read echo response
    data_buffer response = client_conn.read();
    EXPECT_EQ(response.to_string(), "Hello from client!");

    server_thread.join();
    cleanup_socket_library();
}

/**
 * @test MULTITHREADING: Test multiple concurrent std::shared_ptr<connection>s
 * Verifies thread safety when handling multiple std::shared_ptr<connection>s simultaneously
 */
TEST(ConnectionTest, MultithreadedMultipleConnections) {
    initialize_socket_library();

    // Create server
    cppress::sockets::socket server_sock(family::ipv4(), socket::type::stream);
    socket_address server_addr(ip_address("127.0.0.1"), get_random_free_port(), family::ipv4());
    server_sock.bind(server_addr);
    server_sock.listen();

    const int NUM_CLIENTS = 10;
    std::atomic<int> successful_connections{0};
    std::mutex connections_mutex;
    std::vector<std::thread> client_threads;

    // Server thread to accept multiple std::shared_ptr<connection>s
    std::thread server_thread([&]() {
        std::vector<std::thread> handler_threads;

        for (int i = 0; i < NUM_CLIENTS; ++i) {
            std::shared_ptr<connection> client_conn = server_sock.accept();

            // Handle each std::shared_ptr<connection> in separate thread
            handler_threads.emplace_back(
                [conn = std::move(client_conn), &successful_connections]() mutable {
                    try {
                        data_buffer request = conn->read();
                        data_buffer response("Server received: ");
                        response.append(request);
                        conn->write(response);

                        successful_connections++;
                    } catch (...) {
                        // Connection handling failed
                    }
                });
        }

        // Wait for all handlers to complete
        for (auto& t : handler_threads) {
            if (t.joinable())
                t.join();
        }
    });

    // Give server time to start listening
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Create multiple client std::shared_ptr<connection>s
    for (int i = 0; i < NUM_CLIENTS; ++i) {
        client_threads.emplace_back([&server_addr, i]() {
            try {
                connection conn(server_addr);

                data_buffer request("Client ");
                request.append(std::to_string(i));
                conn.write(request);

                data_buffer response = conn.read();
                std::string resp_str = response.to_string();
                EXPECT_TRUE(resp_str.find("Client " + std::to_string(i)) != std::string::npos);
            } catch (...) {
            }
        });
    }

    // Wait for all clients
    for (auto& t : client_threads) {
        if (t.joinable())
            t.join();
    }

    server_thread.join();

    EXPECT_EQ(successful_connections.load(), NUM_CLIENTS);

    cleanup_socket_library();
}
