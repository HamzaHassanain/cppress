#include <regex>
#include <string>

#include "../../../shared/includes/utils.hpp"
#include "types.hpp"

namespace cppress::core {

class router {
    std::vector<std::pair<route, std::vector<request_handler_t>>> routes;
    std::vector<request_handler_t> middlewares;
    std::vector<request_handler_t> not_found_handlers;
    std::vector<request_handler_t> error_handlers;

    std::string prefix;

public:
    router(const std::string& prefix = "") : prefix(prefix) {}


    void use(const request_handler_t& middleware) { middlewares.push_back(middleware); }

    void error(const request_handler_t& error_handler) { error_handlers.push_back(error_handler); }

    void not_found(const request_handler_t& not_found_handler) {
        not_found_handlers.push_back(not_found_handler);
    }

    void get(const std::regex& path_regex, const std::vector<request_handler_t>& handlers) {
        add_route(shared::methods::GET, path_regex, handlers);
    }

    void post(const std::regex& path_regex, const std::vector<request_handler_t>& handlers) {
        add_route(shared::methods::POST, path_regex, handlers);
    }

    void put(const std::regex& path_regex, const std::vector<request_handler_t>& handlers) {
        add_route(shared::methods::PUT, path_regex, handlers);
    }

    void delete_(const std::regex& path_regex, const std::vector<request_handler_t>& handlers) {
        add_route(shared::methods::DELETE_, path_regex, handlers);
    }

    void add_route(const std::string& method, const std::regex& path_regex,
                   const std::vector<request_handler_t>& handlers) {
        routes.emplace_back(route(method, path_regex), handlers);
    }

    virtual ~router() = default;
};

}  // namespace cppress::core
