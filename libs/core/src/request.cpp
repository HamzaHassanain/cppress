#include "../includes/request.hpp"

#include <stdexcept>

#include "../../../shared/includes/utils.hpp"
namespace cppress::core {
request::request(const std::string& method, const std::string& uri, const std::string& version,
                 const std::multimap<std::string, std::string>& headers, const std::string& body,
                 std::function<void()> close_connection)
    : method(method),
      uri(uri),
      version(version),
      headers(headers),
      body(body),
      close_connection(close_connection) {
    std::multimap<std::string, std::string> lower_case_headers;
    for (const auto& header : headers) {
        lower_case_headers.insert({shared::to_uppercase(header.first), header.second});
    }
    this->headers = std::move(lower_case_headers);
}

request::request(request&& other)
    : method(std::move(other.method)),
      uri(std::move(other.uri)),
      version(std::move(other.version)),
      headers(std::move(other.headers)),
      body(std::move(other.body)),
      close_connection(std::move(other.close_connection)) {}

request& request::operator=(request&& other) {
    if (this != &other) {
        method = std::move(other.method);
        uri = std::move(other.uri);
        version = std::move(other.version);
        headers = std::move(other.headers);
        body = std::move(other.body);
        close_connection = std::move(other.close_connection);
    }
    return *this;
}

std::string request::get_method() const {
    return method;
}

std::string request::get_uri() const {
    return uri;
}

std::string request::get_version() const {
    return version;
}

std::vector<std::string> request::get_header(const std::string& name) const {
    std::vector<std::string> values;
    auto range = headers.equal_range(shared::to_uppercase(name));
    for (auto it = range.first; it != range.second; ++it) {
        values.push_back(it->second);
    }
    return values;
}

std::vector<std::pair<std::string, std::string>> request::get_headers() const {
    std::vector<std::pair<std::string, std::string>> headers_vector;
    for (const auto& header : headers) {
        headers_vector.emplace_back(shared::to_uppercase(header.first), header.second);
    }
    return headers_vector;
}

std::string request::get_body() const {
    return body;
}

bool request::has_header(const std::string& name) const {
    return headers.find(shared::to_uppercase(name)) != headers.end();
}

std::string request::get_path() const {
    size_t query_pos = uri.find('?');
    if (query_pos != std::string::npos) {
        return uri.substr(0, query_pos);
    }
    return uri;
}

std::map<std::string, std::string> request::get_query_params() const {
    std::map<std::string, std::string> params;
    size_t query_pos = uri.find('?');

    if (query_pos == std::string::npos || query_pos == uri.length() - 1) {
        return params;  // No query string
    }

    std::string query_string = uri.substr(query_pos + 1);
    size_t start = 0;

    while (start < query_string.length()) {
        size_t amp_pos = query_string.find('&', start);
        size_t end = (amp_pos != std::string::npos) ? amp_pos : query_string.length();

        std::string param = query_string.substr(start, end - start);
        size_t eq_pos = param.find('=');

        if (eq_pos != std::string::npos) {
            std::string key = param.substr(0, eq_pos);
            std::string value = param.substr(eq_pos + 1);

            // Simple URL decoding for common cases
            // Replace '+' with space
            for (char& c : value) {
                if (c == '+')
                    c = ' ';
            }

            params[key] = value;
        }

        start = end + 1;
    }

    return params;
}

std::string request::get_query_param(const std::string& name) const {
    auto params = get_query_params();
    auto it = params.find(name);
    return (it != params.end()) ? it->second : "";
}
}  // namespace cppress::core