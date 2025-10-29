
#include <iostream>
#include <map>
#include <thread>

#include "includes/data_buffer.hpp"
#include "includes/epoll_server.hpp"
#include "includes/socket.hpp"
#include "includes/socket_address.hpp"
#include "includes/utilities.hpp"

class EchoServer : public cppress::sockets::epoll_server {
public:
    EchoServer() : cppress::sockets::epoll_server(1000) {}

protected:
    // Per-connection state: accumulate data across multiple on_data_available calls
    std::map<int, std::vector<char>> connection_buffers;  // Use vector for efficient append
    std::map<int, size_t> expected_content_length;
    std::map<int, bool> headers_parsed;

    void on_connection_opened(std::shared_ptr<cppress::sockets::connection> conn) override {
        int fd = conn->native_handle();
        connection_buffers[fd].reserve(10 * 1024 * 1024);  // Pre-allocate 10MB
        expected_content_length[fd] = 0;
        headers_parsed[fd] = false;
    }

    void on_data_available(std::shared_ptr<cppress::sockets::connection> conn) override {
        try {
            int fd = conn->native_handle();

            // Read ALL available data in this event (edge-triggered)
            while (true) {
                cppress::sockets::data_buffer db = conn->read();
                if (db.size() == 0)
                    break;  // No more data available RIGHT NOW (not EOF!)

                // Efficient append using vector - avoids string reallocation
                auto& buffer = connection_buffers[fd];
                buffer.insert(buffer.end(), db.data(), db.data() + db.size());
            }

            // Parse headers if not done yet
            if (!headers_parsed[fd]) {
                auto& buffer = connection_buffers[fd];
                std::string_view view(buffer.data(), buffer.size());
                size_t header_end = view.find("\r\n\r\n");
                if (header_end != std::string::npos) {
                    headers_parsed[fd] = true;

                    // Extract Content-Length from headers
                    std::string_view headers = view.substr(0, header_end);
                    size_t cl_pos = headers.find("Content-Length:");
                    if (cl_pos != std::string::npos) {
                        size_t start = cl_pos + 15;
                        size_t end = headers.find("\r\n", start);
                        std::string_view length_str = headers.substr(start, end - start);
                        // Trim whitespace
                        while (!length_str.empty() &&
                               (length_str[0] == ' ' || length_str[0] == '\t'))
                            length_str.remove_prefix(1);
                        expected_content_length[fd] = std::stoull(std::string(length_str));
                    }
                }
            }

            // Check if we have received the complete request
            if (headers_parsed[fd]) {
                auto& buffer = connection_buffers[fd];
                std::string_view view(buffer.data(), buffer.size());
                size_t header_end = view.find("\r\n\r\n");
                size_t total_expected = header_end + 4 + expected_content_length[fd];

                if (buffer.size() >= total_expected) {
                    // Complete request received!
                    size_t received_body_size = buffer.size() - (header_end + 4);

                    // Send response
                    std::string response =
                        "{\"length\": " + std::to_string(received_body_size) + "}\n";
                    std::string http_response =
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: application/json\r\n"
                        "Content-Length: " +
                        std::to_string(response.size()) +
                        "\r\n"
                        "Connection: keep-alive\r\n"
                        "\r\n" +
                        response;
                    send_message(conn, cppress::sockets::data_buffer(http_response));

                    // Clear buffers for next request on same connection
                    buffer.clear();
                    expected_content_length[fd] = 0;
                    headers_parsed[fd] = false;
                }
            }
        } catch (const std::exception& e) {
            on_exception_occurred(e);
            close_connection(conn);
        }
    }

    void on_connection_closed(std::shared_ptr<cppress::sockets::connection> conn) override {
        // Clean up connection state
        int fd = conn->native_handle();
        connection_buffers.erase(fd);
        expected_content_length.erase(fd);
        headers_parsed.erase(fd);
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