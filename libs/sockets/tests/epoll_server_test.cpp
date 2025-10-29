/**
 * @file test_std::shared_ptr<connection>.cpp
 * @brief Unit tests for the std::shared_ptr<connection> class
 */

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>

#include "../includes.hpp"

using namespace cppress::sockets;

TEST(EpollServerTest, BasicEchoServer) {
    initialize_socket_library();

    epoll_server server(1024);

    auto listener = make_listener_socket(9990, ("127.0.0.1"));
    ASSERT_TRUE(listener != nullptr);

    ASSERT_TRUE(server.register_listener_socket(listener));

    std::thread server_thread([&server]() { server.listen(1000); });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    try {
        connection client1, client2;
        client1.connect(socket_address(port(9990), ip_address("127.0.0.1")));
        client2.connect(socket_address(port(9990), ip_address("127.0.0.1")));

        std::string msg1 = "Hello, Server!";
        client1.write(data_buffer(msg1));

        auto response1 = client1.read();
        std::string echo1 = response1.to_string();

        EXPECT_NE(echo1.find(msg1), std::string::npos);

        std::string msg2 = "Second message";
        client2.write(data_buffer(msg2));

        auto response2 = client2.read();
        std::string echo2 = response2.to_string();

        EXPECT_NE(echo2.find(msg2), std::string::npos);

        client1.close();
        client2.close();

    } catch (const std::exception& e) {
        FAIL() << "Client error: " << e.what();
    }

    server.shutdown();
    server_thread.join();

    cleanup_socket_library();
}

TEST(EpollServerTest, TestConnectionPersistance) {
    initialize_socket_library();

    epoll_server server(1024);

    auto listener = make_listener_socket((9991), ("127.0.0.1"));
    ASSERT_TRUE(listener != nullptr);

    ASSERT_TRUE(server.register_listener_socket(listener));

    std::thread server_thread([&server]() { server.listen(1000); });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::vector<std::thread> client_threads;
    for (int client_cnt = 1; client_cnt <= 5; client_cnt++) {
        client_threads.emplace_back([client_cnt]() {
            connection persistent_conn;
            persistent_conn.connect(socket_address(port(9991), ip_address("127.0.0.1")));

            for (int i = 0; i < 10; i++) {
                std::string msg =
                    "Client " + std::to_string(client_cnt) + " - Message " + std::to_string(i);
                persistent_conn.write(msg);

                auto resp = persistent_conn.read();
                std::string echo = resp.to_string();
                EXPECT_NE(echo.find(msg), std::string::npos);
            }

            persistent_conn.close();
        });
    }
    for (auto& t : client_threads) {
        t.join();
    }
    server.shutdown();
    server_thread.join();

    cleanup_socket_library();
}
