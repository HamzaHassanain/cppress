#include "../includes/http_request_parser.hpp"

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>

namespace cppress::http {

http_parse_result http_request_parser::parse(std::shared_ptr<cppress::sockets::connection> conn,
                                             const cppress::sockets::data_buffer& data) {
    std::lock_guard<std::mutex> lock(parser_mutex_);

    auto connection_id = conn->remote_endpoint().to_string();

    if (pending_requests_.find(connection_id) != pending_requests_.end()) {
        return continue_parsing(pending_requests_[connection_id], data);
    }

    return begin_parsing(connection_id, data, conn->native_handle());
}

http_parse_result http_request_parser::continue_parsing(http_parse_state& state,
                                                        const cppress::sockets::data_buffer& data) {
    state.last_activity = std::chrono::steady_clock::now();

    if (state.strategy != parse_strategy::CONTENT_LENGTH) {
        return http_parse_result(true, "UNSUPPORTED_PARSE_STRATEGY", state.uri, state.http_version,
                                 {}, "");
    } else  // Content-Length handling
    {
        return accumulate_body_data(state, data);
    }
}

http_parse_result http_request_parser::begin_parsing(const std::string& connection_id,
                                                     const cppress::sockets::data_buffer& data,
                                                     int socket_fd) {
    // Convert raw message to string stream for line-by-line parsing
    std::istringstream request_stream(data.to_string());

    // HTTP request components to be parsed
    std::string method, uri, version;

    // Parse request line
    auto [request_line_valid, error_message] =
        parse_request_line(request_stream, method, uri, version);
    if (!request_line_valid) {
        return http_parse_result(true, error_message, uri, version, {}, "");
    }

    // Parse headers
    auto [headers_valid, headers] = parse_headers(request_stream, uri, version);
    if (!headers_valid) {
        return http_parse_result(true, "BAD_HEADERS_TOO_LARGE", uri, version, {}, "");
    }

    std::size_t content_length = 0;
    auto content_length_it = headers.find(cppress::sockets::to_uppercase("content-length"));
    auto transfer_encoding = headers.find(cppress::sockets::to_uppercase("Transfer-Encoding"));

    bool has_transfer_encoding = (transfer_encoding != headers.end()) &&
                                 has_chunked_encoding(headers.equal_range(
                                     cppress::sockets::to_uppercase("Transfer-Encoding")));

    bool has_content_length = (content_length_it != headers.end());

    if (headers.count(cppress::sockets::to_uppercase("content-length")) > 1 ||
        (has_content_length && has_transfer_encoding)) {
        return http_parse_result(true, "BAD_REPEATED_LENGTH_OR_TRANSFER_ENCODING_OR_BOTH", uri,
                                 version, headers, "");
    }

    if (has_content_length) {
        content_length = std::stoull(content_length_it->second);
        return parse_content_length_body(connection_id, request_stream, method, uri, version,
                                         headers, content_length, socket_fd);
    } else if (has_transfer_encoding) {
        // return parse_chunked_encoding_body(connection_id, request_stream, method, uri, version,
        //                                    headers, socket_fd);
        return http_parse_result(true, "UNSUPPORTED_TRANSFER_ENCODING_CHUNKED", uri, version,
                                 headers, "");
    }

    // No body to process
    return http_parse_result(true, method, uri, version, headers, "");
}

void http_request_parser::cleanup_idle_connections(std::chrono::seconds max_idle_time,
                                                   std::function<void(int)> close_connection) {
    std::lock_guard<std::mutex> lock(parser_mutex_);
    auto now = std::chrono::steady_clock::now();
    for (auto it = pending_requests_.begin(); it != pending_requests_.end();) {
        auto duration =
            std::chrono::duration_cast<std::chrono::seconds>(now - it->second.last_activity);
        if (duration > max_idle_time) {
            close_connection(it->second.socket_fd);
            it = pending_requests_.erase(it);
        } else {
            ++it;
        }
    }
}

std::pair<bool, std::string> http_request_parser::parse_request_line(
    std::istringstream& request_stream, std::string& method, std::string& uri,
    std::string& version) {
    std::string line;
    if (std::getline(request_stream, line) && !line.empty()) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        std::istringstream request_line(line);
        request_line >> method >> uri >> version;
    }

    if (method.empty() || uri.empty() || version.empty()) {
        return {false, "BAD_METHOD_OR_URI_OR_VERSION"};
    }

    return {true, ""};
}

std::pair<bool, std::multimap<std::string, std::string>> http_request_parser::parse_headers(
    std::istringstream& request_stream, const std::string& uri, const std::string& version) {
    std::multimap<std::string, std::string> headers;
    std::size_t headers_size = 0;
    std::string line;

    while (std::getline(request_stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line.empty()) {
            break;
        }

        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string header_name = line.substr(0, colon_pos);
            std::string header_value = line.substr(colon_pos + 1);

            size_t start = header_value.find_first_not_of(" \t");
            if (start != std::string::npos) {
                header_value = header_value.substr(start);
            }

            size_t end = header_value.find_last_not_of(" \t");
            if (end != std::string::npos) {
                header_value = header_value.substr(0, end + 1);
            }
            headers_size += header_name.size() + header_value.size();

            if (headers_size > config::MAX_HEADER_SIZE) {
                return {false, {}};
            }
            headers.emplace(cppress::sockets::to_uppercase(header_name), header_value);
        }
    }

    return {true, headers};
}

bool http_request_parser::has_chunked_encoding(
    const std::pair<std::multimap<std::string, std::string>::iterator,
                    std::multimap<std::string, std::string>::iterator>& range) {
    for (auto it = range.first; it != range.second; ++it) {
        auto tmp = it->second;
        std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
        if (tmp.find("chunked") != std::string::npos) {
            return true;
        }
    }
    return false;
}

http_parse_result http_request_parser::parse_content_length_body(
    const std::string& connection_id, std::istringstream& request_stream, const std::string& method,
    const std::string& uri, const std::string& version,
    const std::multimap<std::string, std::string>& headers, size_t content_length, int socket_fd) {
    // Read the body from the stream
    std::ostringstream body_stream;
    body_stream << request_stream.rdbuf();
    std::string body = body_stream.str();

    // Complete request in one go
    if (body.size() == content_length) {
        return http_parse_result(true, method, uri, version, headers, body);
    } else if (body.size() > content_length || body.size() > config::MAX_BODY_SIZE) {
        return http_parse_result(true, "BAD_CONTENT_TOO_LARGE", uri, version, headers, "");
    } else {
        // Need to continue handling in subsequent calls
        pending_requests_.insert(
            {connection_id, http_parse_state(connection_id, parse_strategy::CONTENT_LENGTH)});
        auto& state_ref = pending_requests_[connection_id];
        state_ref.expected_body_length = content_length;
        state_ref.accumulated_body = body;
        state_ref.method = method;
        state_ref.uri = uri;
        state_ref.http_version = version;
        state_ref.headers = headers;
        state_ref.last_activity = std::chrono::steady_clock::now();
        state_ref.socket_fd = socket_fd;
        return http_parse_result(false, method, uri, version, headers, body);
    }
}

http_parse_result http_request_parser::accumulate_body_data(
    http_parse_state& state, const cppress::sockets::data_buffer& data) {
    std::string body = data.to_string();
    state.accumulated_body += body;

    if (state.accumulated_body.size() > config::MAX_BODY_SIZE) {
        return http_parse_result(true, "BAD_CONTENT_TOO_LARGE", state.uri, state.http_version,
                                 state.headers, "");
    }
    if (state.accumulated_body.size() == state.expected_body_length) {
        auto return_value = http_parse_result(true, state.method, state.uri, state.http_version,
                                              state.headers, state.accumulated_body);
        pending_requests_.erase(state.connection_id);
        return return_value;
    }

    if (state.accumulated_body.size() > state.expected_body_length ||
        state.accumulated_body.size() > config::MAX_BODY_SIZE) {
        return http_parse_result(true, "BAD_CONTENT_TOO_LARGE", state.uri, state.http_version,
                                 state.headers, "");
    }

    return http_parse_result(false, state.method, state.uri, state.http_version, {}, "");
}

}  // namespace cppress::http
