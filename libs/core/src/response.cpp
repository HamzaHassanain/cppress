
#include "../includes/response.hpp"

#include <ctime>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>

#include "../../../shared/includes/utils.hpp"

namespace cppress::core {
response::response(std::function<void()> close_connection,
                   std::function<void(const std::string&)> send_message)
    : close_connection(close_connection), send_message(send_message) {
    std::multimap<std::string, std::string> upper_case_headers;

    for (const auto& header : headers) {
        upper_case_headers.insert({shared::to_uppercase(header.first), header.second});
    }
    this->headers = std::move(upper_case_headers);
}

response::response(response&& other)
    : version(std::move(other.version)),
      status_code(other.status_code),
      status_message(std::move(other.status_message)),
      headers(std::move(other.headers)),
      trailers(std::move(other.trailers)),
      body(std::move(other.body)),
      close_connection(std::move(other.close_connection)),
      send_message(std::move(other.send_message)) {
    other.status_code = 0;             // Invalidate the moved-from response
    other.send_message = nullptr;      // Reset the moved-from send_message
    other.close_connection = nullptr;  // Reset the moved-from close_connection
}

response& response::operator=(response&& other) {
    if (this != &other) {
        version = std::move(other.version);
        status_code = other.status_code;
        status_message = std::move(other.status_message);
        headers = std::move(other.headers);
        trailers = std::move(other.trailers);
        body = std::move(other.body);
        close_connection = std::move(other.close_connection);
        send_message = std::move(other.send_message);

        other.status_code = 0;             // Invalidate the moved-from response
        other.send_message = nullptr;      // Reset the moved-from send_message
        other.close_connection = nullptr;  // Reset the moved-from close_connection
    }
    return *this;
}

bool response::validate() const {
    return true;
}

std::string get_current_date() {
    // Get current time
    std::time_t now = std::time(nullptr);
    std::tm tm = *std::gmtime(&now);

    // Format date according to RFC 1123
    char buffer[30];
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", &tm);
    return std::string(buffer);
}

std::string response::to_string() const {
    std::ostringstream response_stream;
    response_stream << version << " " << status_code << " " << status_message << "\r\n";
    response_stream << "Date: " << get_current_date() << "\r\n";

    // Add Content-Length header if not already present (critical for HTTP/1.1 persistent
    // connections)
    if (headers.find("CONTENT-LENGTH") == headers.end() &&
        headers.find("TRANSFER-ENCODING") == headers.end()) {
        response_stream << "Content-Length: " << body.size() << "\r\n";
    }

    // Add all other headers
    for (const auto& header : headers) {
        response_stream << shared::to_uppercase(header.first) << ": " << header.second << "\r\n";
    }

    // Empty line separating headers from body
    response_stream << "\r\n";

    // Add body if present
    if (!body.empty()) {
        response_stream << body;
    }

    return response_stream.str();
}

void response::set_body(const std::string& body) {
    this->body = body;
}

void response::set_status(int status_code, const std::string& status_message) {
    this->status_code = status_code;
    this->status_message = status_message;
}

void response::set_version(const std::string& version) {
    this->version = version;
}

void response::add_trailer(const std::string& name, const std::string& value) {
    trailers.insert({shared::to_uppercase(name), value});
}

void response::add_header(const std::string& name, const std::string& value) {
    headers.insert({shared::to_uppercase(name), value});
}

std::string response::get_body() const {
    return body;
}

std::string response::get_version() const {
    return version;
}

std::string response::get_status_message() const {
    return status_message;
}

int response::get_status_code() const {
    return status_code;
}

std::vector<std::string> response::get_header(const std::string& name) const {
    std::vector<std::string> values;
    auto range = headers.equal_range(shared::to_uppercase(name));
    for (auto it = range.first; it != range.second; ++it) {
        values.push_back(it->second);
    }
    return values;
}

std::vector<std::string> response::get_trailer(const std::string& name) const {
    std::vector<std::string> values;
    auto range = trailers.equal_range(shared::to_uppercase(name));
    for (auto it = range.first; it != range.second; ++it) {
        values.push_back(it->second);
    }
    return values;
}

bool response::has_header(const std::string& name) const {
    return headers.find(shared::to_uppercase(name)) != headers.end();
}

void response::remove_header(const std::string& name) {
    headers.erase(shared::to_uppercase(name));
}

void response::set_header(const std::string& name, const std::string& value) {
    std::string upper_name = shared::to_uppercase(name);
    headers.erase(upper_name);
    headers.insert({upper_name, value});
}

void response::json(const std::string& json_data, int status_code) {
    set_status(status_code, get_status_text(status_code));
    set_header("Content-Type", "application/json; charset=utf-8");
    set_body(json_data);
    send();
}

void response::html(const std::string& html_content, int status_code) {
    set_status(status_code, get_status_text(status_code));
    set_header("Content-Type", "text/html; charset=utf-8");
    set_body(html_content);
    send();
}

void response::text(const std::string& text_content, int status_code) {
    set_status(status_code, get_status_text(status_code));
    set_header("Content-Type", "text/plain; charset=utf-8");
    set_body(text_content);
    send();
}

void response::redirect(const std::string& location, int status_code) {
    set_status(status_code, get_status_text(status_code));
    set_header("Location", location);
    set_body("");
    send();
}

void response::status(int code) {
    set_status(code, get_status_text(code));
}

std::string response::get_status_text(int code) {
    auto it = cppress::shared::http_status_codes.find(code);
    return (it != cppress::shared::http_status_codes.end()) ? it->second : "Unknown";
}

void response::end() {
    try {
        if (validate()) {
            close_connection();
        } else {
            throw std::runtime_error(
                "Invalid HTTP response or client connection may be already closed");
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Error ending HTTP response: " + std::string(e.what()));
    }
}

void response::send() {
    try {
        if (validate()) {
            send_message(to_string());
        } else {
            throw std::runtime_error(
                "Invalid HTTP response or client connection may be already closed");
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Error sending HTTP response:\n" + std::string(e.what()));
    }
}

void response::send_trailers() {
    try {
        if (validate()) {
            std::ostringstream trailer_stream;
            for (const auto& trailer : trailers) {
                trailer_stream << shared::to_uppercase(trailer.first) << ": " << trailer.second
                               << "\r\n";
            }
            send_message(trailer_stream.str());
        } else {
            throw std::runtime_error(
                "Invalid HTTP response or client connection may be already closed");
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Error sending HTTP response:\n" + std::string(e.what()));
    }
}
}  // namespace cppress::core