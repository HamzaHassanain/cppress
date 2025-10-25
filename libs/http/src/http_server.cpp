
#include "../includes/http_server.hpp"

#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>
namespace cppress::http {

http_server::http_server(const cppress::sockets::socket_address& addr, int timeout_milliseconds)
    : cppress::sockets::epoll_server(config::MAX_FILE_DESCRIPTORS) {
    this->timeout_milliseconds = timeout_milliseconds;
    this->server_socket = cppress::sockets::make_listener_socket(
        addr.port().value(), addr.address().string(), config::BACKLOG_SIZE);
    if (!this->server_socket) {
        throw std::runtime_error("Failed to create listener socket");
    }

    this->register_listener_socket(this->server_socket);

    // spin a thread that cleans idle connections each MAX_IDLE_TIME_SECONDS
    std::function<void(int)> close_connection_for_handler = [this](int fd) -> void {
        this->close_connection(fd);
    };
    std::thread([this, close_connection_for_handler]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(config::MAX_IDLE_TIME_SECONDS));
            parser_.cleanup_idle_connections(config::MAX_IDLE_TIME_SECONDS,
                                             close_connection_for_handler);
        }
    }).detach();
}

void http_server::on_message_received(std::shared_ptr<cppress::sockets::connection> conn,
                                      const cppress::sockets::data_buffer& message) {
    auto close_connection_for_objects = [this, conn]() { this->close_connection(conn); };
    auto send_message_for_request = [this, conn](const std::string& message) {
        this->send_message(conn, cppress::sockets::data_buffer(message));
    };

    bool is_complete = false;
    std::string method = "", uri = "", http_version = "", body = "";
    std::multimap<std::string, std::string> headers;
    try {
        auto result = parser_.parse(conn, message);
        is_complete = result.is_complete;
        method = result.method;
        uri = result.uri;
        http_version = result.http_version;
        body = result.body;
        headers = result.headers;

        if (static_cast<int>(headers.size()) >= 0)
            on_headers_received(conn, headers, method, uri, http_version, body);

        if (!is_complete)
            return;
    } catch (const std::exception& e) {
        this->stop_reading_from_connection(conn);

        // Create HTTP request object with parsed data
        http_request request("BAD_REQUEST", uri, http_version, headers, body,
                             close_connection_for_objects);

        // Create HTTP response object with default HTTP/1.1 version
        http_response response("HTTP/1.1", {}, close_connection_for_objects,
                               send_message_for_request);
        this->on_request_received(request, response);
        return;
    }

    // Shall be removed to support persistent connections
    this->stop_reading_from_connection(conn);

    // Create HTTP request object with parsed data
    http_request request(method, uri, http_version, headers, body, close_connection_for_objects);

    // Create HTTP response object with default HTTP/1.1 version
    http_response response("HTTP/1.1", {}, close_connection_for_objects, send_message_for_request);

    // Invoke user-defined request handler with parsed request and response objects
    // User callback populates response and optionally closes connection
    this->on_request_received(request, response);
}

void http_server::on_request_received(http_request& request, http_response& response) {
    if (request_callback) {
        request_callback(request, response);
    } else {
        throw std::runtime_error("No request handler registered");
    }
}

void http_server::on_listen_success() {
    if (listen_success_callback)
        listen_success_callback();
}

void http_server::on_shutdown_success() {
    if (server_shutdown_callback)
        server_shutdown_callback();
}

void http_server::on_exception_occurred(const std::exception& e) {
    if (error_callback)
        error_callback(e);
}

void http_server::on_connection_closed(std::shared_ptr<cppress::sockets::connection> conn) {
    if (client_disconnected_callback)
        client_disconnected_callback(conn);
}

void http_server::on_connection_opened(std::shared_ptr<cppress::sockets::connection> conn) {
    if (client_connected_callback)
        client_connected_callback(conn);
}

void http_server::on_waiting_for_activity() {
    if (waiting_for_activity_callback)
        waiting_for_activity_callback();
}

void http_server::set_request_callback(
    std::function<void(http_request&, http_response&)> callback) {
    request_callback = callback;
}

void http_server::set_listen_success_callback(std::function<void()> callback) {
    listen_success_callback = callback;
}

void http_server::set_server_stopped_callback(std::function<void()> callback) {
    server_shutdown_callback = callback;
}

void http_server::set_error_callback(std::function<void(const std::exception&)> callback) {
    error_callback = callback;
}

void http_server::set_client_connected_callback(
    std::function<void(std::shared_ptr<cppress::sockets::connection>)> callback) {
    client_connected_callback = callback;
}

void http_server::set_client_disconnected_callback(
    std::function<void(std::shared_ptr<cppress::sockets::connection>)> callback) {
    client_disconnected_callback = callback;
}

void http_server::set_waiting_for_activity_callback(std::function<void()> callback) {
    // BUG: Should be waiting_for_activity_callback = callback;
    waiting_for_activity_callback = callback;
}
}  // namespace cppress::http