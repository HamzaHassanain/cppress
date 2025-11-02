#include <functional>
#include <regex>
#include <stdexcept>
#include <string>

#include "../../../shared/includes/utils.hpp"
#include "request.hpp"
#include "response.hpp"

namespace cppress::core {
using cppress::shared::get_url_path;
using cppress::shared::url_decode;

struct route {
private:
    /// @note should always consider using shared::methods constants
    std::string method;

    /// @brief URI pattern as regex to allow flexible matching
    std::regex path_pattern;

public:
    route(const std::string& method_, const std::regex& path_pattern_)
        : method(method_), path_pattern(path_pattern_) {}

    std::string get_method() const { return method; }
    std::regex get_path_pattern() const { return path_pattern; }
    bool matches(const std::string& method_, const std::string& path) const {
        return method == method_ && std::regex_match(get_url_path(url_decode(path)), path_pattern);
    }
};

struct packet_info {
    std::string http_method;
    std::string uri;

    packet_info(const std::string& method, const std::string& uri_)
        : http_method(method), uri(uri_) {}

    std::string to_string() const { return http_method + " " + uri; }
};
struct packet_key {
    int fd;
    packet_key(int fd_) : fd(fd_) {}
    bool operator==(const packet_key& other) const { return fd == other.fd; }
    bool operator<(const packet_key& other) const { return fd < other.fd; }
};

using next_func_t = std::function<void(std::unique_ptr<std::exception>)>;
using request_handler_t =
    std::function<void(std::unique_ptr<request>, std::unique_ptr<response>, next_func_t)>;

};  // namespace cppress::core