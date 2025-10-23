// /**
//  * @file test_epoll_server.cpp
//  * @brief Unit tests for the epoll_server class
//  */

// #include <gtest/gtest.h>

// #include <atomic>
// #include <chrono>
// #include <memory>
// #include <thread>
// #include <vector>

// #include "includes/data_buffer.hpp"
// #include "includes/epoll_server.hpp"
// #include "includes/socket.hpp"
// #include "includes/socket_address.hpp"
// #include "includes/utilities.hpp"

// using namespace cppress::sockets;

// /**
//  * @test Test epoll_server basic start and stop
//  * Verifies server can be started and stopped cleanly
//  */
// TEST(EpollServerTest, BasicStartAndStop) {
//     initialize_socket_library();

//     port server_port = get_random_free_port();

//     // Create epoll server
//     epoll_server server(server_port, family::ipv4());

//     // Start server in separate thread
//     std::atomic<bool> server_started{false};
//     std::thread server_thread([&]() {
//         server_started = true;
//         // Server will run until stopped
//         server.start();
//     });

//     // Give server time to start
//     std::this_thread::sleep_for(std::chrono::milliseconds(200));
//     EXPECT_TRUE(server_started);

//     // Stop the server
//     server.stop();

//     // Wait for server thread to finish
//     if (server_thread.joinable()) {
//         server_thread.join();
//     }

//     cleanup_socket_library();
// }

// /**
//  * @test Test epoll_server accepting connections
//  * Verifies server can accept and handle client connections
//  */
// TEST(EpollServerTest, AcceptClientConnections) {
//     initialize_socket_library();

//     port server_port = get_random_free_port();
//     epoll_server server(server_port, family::ipv4());

//     std::atomic<int> connections_received{0};

//     // Set up connection handler
//     server.on_connection([&connections_received](connection& conn) { connections_received++; });

//     // Start server
//     std::thread server_thread([&]() { server.start(); });

//     // Give server time to start
//     std::this_thread::sleep_for(std::chrono::milliseconds(200));

//     // Create client connections
//     const int NUM_CLIENTS = 3;
//     std::vector<std::thread> client_threads;

//     for (int i = 0; i < NUM_CLIENTS; ++i) {
//         client_threads.emplace_back([&server_port]() {
//             try {
//                 socket client_sock(family::ipv4(), socket::type::stream);
//                 socket_address addr(ip_address("127.0.0.1"), server_port, family::ipv4());
//                 client_sock.connect(addr);

//                 // Keep connection alive briefly
//                 std::this_thread::sleep_for(std::chrono::milliseconds(100));
//             } catch (...) {
//                 // Connection failed
//             }
//         });
//     }

//     // Wait for clients
//     for (auto& t : client_threads) {
//         if (t.joinable())
//             t.join();
//     }

//     // Give server time to process connections
//     std::this_thread::sleep_for(std::chrono::milliseconds(200));

//     // Stop server
//     server.stop();
//     if (server_thread.joinable()) {
//         server_thread.join();
//     }

//     EXPECT_GE(connections_received.load(), NUM_CLIENTS);

//     cleanup_socket_library();
// }

// /**
//  * @test Test epoll_server message handling
//  * Verifies server can receive and process messages
//  */
// TEST(EpollServerTest, MessageHandling) {
//     initialize_socket_library();

//     port server_port = get_random_free_port();
//     epoll_server server(server_port, family::ipv4());

//     std::atomic<int> messages_received{0};
//     std::atomic<bool> correct_message{false};

//     // Set up message handler
//     server.on_message([&](connection& conn, data_buffer& data) {
//         messages_received++;
//         std::string msg = data.to_string();
//         if (msg.find("Test Message") != std::string::npos) {
//             correct_message = true;
//         }

//         // Echo back
//         conn.write(data);
//     });

//     // Start server
//     std::thread server_thread([&]() { server.start(); });

//     // Give server time to start
//     std::this_thread::sleep_for(std::chrono::milliseconds(200));

//     // Create client and send message
//     std::thread client_thread([&server_port, &correct_message]() {
//         try {
//             socket client_sock(family::ipv4(), socket::type::stream);
//             socket_address addr(ip_address("127.0.0.1"), server_port, family::ipv4());
//             client_sock.connect(addr);

//             // Send message
//             data_buffer msg("Test Message");
//             client_sock.send(msg);

//             // Wait for response
//             std::this_thread::sleep_for(std::chrono::milliseconds(100));
//             data_buffer response = client_sock.receive();

//             if (response.to_string() == "Test Message") {
//                 // Message echoed correctly
//             }
//         } catch (...) {
//             // Client failed
//         }
//     });

//     client_thread.join();

//     // Give server time to process
//     std::this_thread::sleep_for(std::chrono::milliseconds(200));

//     // Stop server
//     server.stop();
//     if (server_thread.joinable()) {
//         server_thread.join();
//     }

//     EXPECT_GE(messages_received.load(), 1);
//     EXPECT_TRUE(correct_message);

//     cleanup_socket_library();
// }

// /**
//  * @test MULTITHREADING: Test epoll_server with concurrent clients
//  * Verifies server can handle multiple simultaneous connections
//  */
// TEST(EpollServerTest, MultithreadedConcurrentClients) {
//     initialize_socket_library();

//     port server_port = get_random_free_port();
//     epoll_server server(server_port, family::ipv4());

//     std::atomic<int> total_connections{0};
//     std::atomic<int> total_messages{0};

//     // Set up handlers
//     server.on_connection([&total_connections](connection& conn) { total_connections++; });

//     server.on_message([&total_messages](connection& conn, data_buffer& data) {
//         total_messages++;
//         // Echo back the data
//         conn.write(data);
//     });

//     // Start server
//     std::thread server_thread([&]() { server.start(); });

//     // Give server time to start
//     std::this_thread::sleep_for(std::chrono::milliseconds(300));

//     // Launch multiple concurrent clients
//     const int NUM_CLIENTS = 20;
//     std::atomic<int> successful_clients{0};
//     std::vector<std::thread> client_threads;

//     for (int i = 0; i < NUM_CLIENTS; ++i) {
//         client_threads.emplace_back([&server_port, &successful_clients, i]() {
//             try {
//                 socket client_sock(family::ipv4(), socket::type::stream);
//                 socket_address addr(ip_address("127.0.0.1"), server_port, family::ipv4());
//                 client_sock.connect(addr);

//                 // Send multiple messages
//                 for (int j = 0; j < 3; ++j) {
//                     std::string msg =
//                         "Client " + std::to_string(i) + " Message " + std::to_string(j);
//                     client_sock.send(data_buffer(msg));
//                     std::this_thread::sleep_for(std::chrono::milliseconds(10));
//                 }

//                 successful_clients++;
//             } catch (...) {
//                 // Client failed
//             }
//         });
//     }

//     // Wait for all clients
//     for (auto& t : client_threads) {
//         if (t.joinable())
//             t.join();
//     }

//     // Give server time to process all messages
//     std::this_thread::sleep_for(std::chrono::milliseconds(500));

//     // Stop server
//     server.stop();
//     if (server_thread.joinable()) {
//         server_thread.join();
//     }

//     // Verify concurrent operations succeeded
//     EXPECT_GE(total_connections.load(), NUM_CLIENTS);
//     EXPECT_GE(successful_clients.load(), NUM_CLIENTS / 2);  // At least half successful

//     cleanup_socket_library();
// }

// /**
//  * @test MULTITHREADING: Test epoll_server thread safety under load
//  * Verifies server maintains thread safety with high concurrent load
//  */
// TEST(EpollServerTest, MultithreadedHighLoadThreadSafety) {
//     initialize_socket_library();

//     port server_port = get_random_free_port();
//     epoll_server server(server_port, family::ipv4());

//     std::atomic<int> connection_count{0};
//     std::atomic<int> message_count{0};
//     std::atomic<int> error_count{0};

//     // Set up handlers with thread-safe counters
//     server.on_connection([&connection_count](connection& conn) {
//         connection_count.fetch_add(1, std::memory_order_relaxed);
//     });

//     server.on_message([&message_count, &error_count](connection& conn, data_buffer& data) {
//         try {
//             message_count.fetch_add(1, std::memory_order_relaxed);

//             // Process and echo
//             std::string response = "ECHO: " + data.to_string();
//             conn.write(data_buffer(response));
//         } catch (...) {
//             error_count.fetch_add(1, std::memory_order_relaxed);
//         }
//     });

//     // Start server
//     std::thread server_thread([&]() { server.start(); });

//     std::this_thread::sleep_for(std::chrono::milliseconds(300));

//     // Create high load with many concurrent clients
//     const int NUM_CLIENTS = 30;
//     const int MESSAGES_PER_CLIENT = 5;
//     std::vector<std::thread> client_threads;
//     std::atomic<int> messages_sent{0};

//     for (int i = 0; i < NUM_CLIENTS; ++i) {
//         client_threads.emplace_back([&, i]() {
//             try {
//                 socket client_sock(family::ipv4(), socket::type::stream);
//                 socket_address addr(ip_address("127.0.0.1"), server_port, family::ipv4());
//                 client_sock.connect(addr);

//                 // Rapid-fire messages
//                 for (int j = 0; j < MESSAGES_PER_CLIENT; ++j) {
//                     std::string msg = "Load test " + std::to_string(i) + "-" + std::to_string(j);
//                     client_sock.send(data_buffer(msg));
//                     messages_sent.fetch_add(1, std::memory_order_relaxed);

//                     // Small delay to avoid overwhelming
//                     std::this_thread::sleep_for(std::chrono::milliseconds(5));
//                 }

//                 // Keep connection alive briefly
//                 std::this_thread::sleep_for(std::chrono::milliseconds(50));
//             } catch (...) {
//                 // Client connection failed
//             }
//         });
//     }

//     // Wait for all clients
//     for (auto& t : client_threads) {
//         if (t.joinable())
//             t.join();
//     }

//     // Give server time to process everything
//     std::this_thread::sleep_for(std::chrono::milliseconds(1000));

//     // Stop server
//     server.stop();
//     if (server_thread.joinable()) {
//         server_thread.join();
//     }

//     // Verify thread safety - no errors should occur
//     EXPECT_EQ(error_count.load(), 0);

//     // Verify reasonable processing occurred
//     EXPECT_GE(connection_count.load(), NUM_CLIENTS / 2);
//     EXPECT_GE(message_count.load(), messages_sent.load() / 4);  // At least 25% processed

//     cleanup_socket_library();
// }
