#include "../../../shared/includes/thread_pool.hpp"
#include "../../sockets/includes.hpp"
#include "consts.hpp"
#include "types.hpp"

namespace cppress::core {

using cppress::sockets::connection;
class server : public cppress::sockets::epoll_server {
    cppress::shared::thread_pool thread_pool;

    std::function<void(std::shared_ptr<connection>)> on_connection_opened_callback;
    std::function<void(std::shared_ptr<connection>)> on_connection_closed_callback;
    std::function<void(std::shared_ptr<connection>)> on_data_available_callback;
    std::function<void()> on_listen_success_callback;
    std::function<void()> on_shutdown_success_callback;
    std::function<void()> on_waiting_for_activity_callback;
    std::function<void(const std::exception&)> on_connection_error_callback;

protected:
    void on_connection_opened(std::shared_ptr<connection> conn) override {
        // Handle new connection opened
    }
    void on_connection_opened(std::shared_ptr<connection> conn) override {
        // Handle new connection opened
    }

    void on_connection_closed(std::shared_ptr<connection> conn) override {
        // Handle connection closed
    }

    void on_data_available(std::shared_ptr<connection> conn) override {
        // Handle data available on connection
    }

    void on_listen_success() override {
        // Handle successful listen
    }

    void on_shutdown_success() override {
        // Handle successful shutdown
    }

    void on_waiting_for_activity() override {
        // Handle waiting for activity
    }

    void on_connection_error(const std::exception& e) override {
        // Handle connection error
    }

    void on_data_available(std::shared_ptr<connection> conn) override {}

public:
    server(std::size_t thread_pool_size = std::max(4u, std::thread::hardware_concurrency()))
        : cppress::sockets::epoll_server(config::MAX_FILE_DESCRIPTORS),
          thread_pool(thread_pool_size) {}
};

}  // namespace cppress::core

/*
namespace {
constexpr size_t INITIAL_BUFFER_SIZE = 5 * 1024 * 1024 + 5 * 1024;
constexpr size_t CONTENT_LENGTH_OFFSET = 15;
constexpr size_t HEADER_DELIMITER_SIZE = 4;
constexpr int MAX_CONNECTIONS = 1000;
constexpr int THREAD_COUNT = 4;
constexpr int SERVER_PORT = 8080;
const std::string HEADER_DELIMITER = "\r\n\r\n";
const std::string CONTENT_LENGTH_HEADER = "Content-Length:";
}  // namespace

struct connection_state {
    std::vector<char> buffer;
    size_t expected_content_length = 0;
    bool headers_parsed = false;

    connection_state() { buffer.reserve(INITIAL_BUFFER_SIZE); }
};
class echo_server : public cppress::sockets::epoll_server {
public:
    echo_server() : cppress::sockets::epoll_server(MAX_CONNECTIONS) {}

protected:
    std::map<int, connection_state> connections;

    void on_connection_opened(std::shared_ptr<cppress::sockets::connection> conn) override {
        int fd = conn->native_handle();
        connections[fd] = connection_state();
    }

    void on_data_available(std::shared_ptr<cppress::sockets::connection> conn) override {
        try {
            int fd = conn->native_handle();
            read_available_data(conn, fd);

            if (!connections[fd].headers_parsed) {
                parse_headers(fd);
            }

            if (connections[fd].headers_parsed && is_request_complete(fd)) {
                handle_complete_request(conn, fd);
            }
        } catch (const std::exception& e) {
            on_connection_error(e);
            close_connection(conn);
        }
    }

    void on_connection_closed(std::shared_ptr<cppress::sockets::connection> conn) override {
        connections.erase(conn->native_handle());
    }

    void on_connection_error(const std::exception& e) override {
        std::cerr << "Server error: " << e.what() << std::endl;
    }

    void on_listen_success() override {
        std::cout << "Echo server started successfully!" << std::endl;
    }

    void on_shutdown_success() override { std::cout << "Server shutdown complete." << std::endl; }

    void on_waiting_for_activity() override {}

private:
    void read_available_data(std::shared_ptr<cppress::sockets::connection> conn, int fd) {
        while (true) {
            cppress::sockets::data_buffer data = conn->read();
            if (data.size() == 0) {
                break;
            }

            auto& buffer = connections[fd].buffer;
            buffer.insert(buffer.end(), data.data(), data.data() + data.size());
        }
    }

    void parse_headers(int fd) {
        auto& state = connections[fd];
        std::string_view view(state.buffer.data(), state.buffer.size());

        size_t header_end = view.find(HEADER_DELIMITER);
        if (header_end == std::string::npos) {
            return;
        }

        state.headers_parsed = true;

        auto content_length = extract_content_length(view.substr(0, header_end));
        if (content_length.has_value()) {
            state.expected_content_length = content_length.value();
        }
    }

    std::optional<size_t> extract_content_length(std::string_view headers) {
        size_t position = headers.find(CONTENT_LENGTH_HEADER);
        if (position == std::string::npos) {
            return std::nullopt;
        }

        size_t start = position + CONTENT_LENGTH_OFFSET;
        size_t end = headers.find("\r\n", start);
        std::string_view length_str = headers.substr(start, end - start);

        length_str = trim_whitespace(length_str);
        return std::stoull(std::string(length_str));
    }

    std::string_view trim_whitespace(std::string_view str) {
        while (!str.empty() && (str[0] == ' ' || str[0] == '\t')) {
            str.remove_prefix(1);
        }
        return str;
    }

    bool is_request_complete(int fd) {
        auto& state = connections[fd];
        std::string_view view(state.buffer.data(), state.buffer.size());

        size_t header_end = view.find(HEADER_DELIMITER);
        size_t total_expected = header_end + HEADER_DELIMITER_SIZE + state.expected_content_length;

        return state.buffer.size() >= total_expected;
    }

    void handle_complete_request(std::shared_ptr<cppress::sockets::connection> conn, int fd) {
        auto& state = connections[fd];

        size_t body_size = calculate_body_size(state);
        std::string response = build_http_response(body_size);

        send_message(conn, cppress::sockets::data_buffer(response));
        reset_connection_state(fd);
    }

    size_t calculate_body_size(const connection_state& state) {
        std::string_view view(state.buffer.data(), state.buffer.size());
        size_t header_end = view.find(HEADER_DELIMITER);
        return state.buffer.size() - (header_end + HEADER_DELIMITER_SIZE);
    }

    std::string build_http_response(size_t body_size) {
        std::string json_body = "{\"length\": " + std::to_string(body_size) + "}\n";

        return "HTTP/1.1 200 OK\r\n"
               "Content-Type: application/json\r\n"
               "Content-Length: " +
               std::to_string(json_body.size()) +
               "\r\n"
               "Connection: keep-alive\r\n"
               "\r\n" +
               json_body;
    }

    void reset_connection_state(int fd) {
        auto& state = connections[fd];
        state.buffer.clear();
        state.expected_content_length = 0;
        state.headers_parsed = false;
    }
};

*/