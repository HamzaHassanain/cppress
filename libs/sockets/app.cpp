
#include <iostream>
#include <thread>

#include "includes/data_buffer.hpp"
#include "includes/epoll_server.hpp"
#include "includes/socket.hpp"
#include "includes/socket_address.hpp"
#include "includes/utilities.hpp"

class EchoServer : public cppress::sockets::epoll_server {
public:
    EchoServer() : cppress::sockets::epoll_server(1000) {}  // Max 1000 connections

protected:
    void on_connection_opened(std::shared_ptr<cppress::sockets::connection> conn) override {
        std::cout << "Client connected from: " << conn->remote_endpoint().to_string() << std::endl;
    }

    void on_message_received(std::shared_ptr<cppress::sockets::connection> conn,
                             const cppress::sockets::data_buffer& message) override {
        std::cout << std::this_thread::get_id() << " ";
        std::cout << "Received: " << message.to_string() << std::endl;
        // Echo the message back
        send_message(conn, message);
        close_connection(conn);
    }

    void on_connection_closed(std::shared_ptr<cppress::sockets::connection> conn) override {
        std::cout << "Client disconnected: " << conn->remote_endpoint().to_string() << std::endl;
    }

    void on_exception_occurred(const std::exception& e) override {
        std::cerr << "Server error: " << e.what() << std::endl;
    }

    void on_listen_success() override {
        std::cout << "Echo server started successfully!" << std::endl;
    }

    void on_shutdown_success() override { std::cout << "Server shutdown complete." << std::endl; }

    void on_waiting_for_activity() override {
        // Optional: periodic maintenance tasks
    }
};

int main() {
    try {
        if (!cppress::sockets::initialize_socket_library()) {
            std::cerr << "Failed to initialize socket library." << std::endl;
            return 1;
        }
        std::vector<std::thread> server_threads;

        // Create TCP listening socket
        for (int i = 0; i < 4; ++i) {
            server_threads.emplace_back([]() {
                auto listener = cppress::sockets::make_listener_socket(8080);
                EchoServer server;
                if (server.register_listener_socket(listener)) {
                    server.listen(1000);  // Start the server event loop
                }
            });
        }

        for (auto& t : server_threads) {
            if (t.joinable())
                t.join();
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    cppress::sockets::cleanup_socket_library();
    return 0;
}