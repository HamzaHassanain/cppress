#pragma once
#include <memory>
#include <string>

#include "../../sockets/includes.hpp"

namespace cppress::http {
class http_connection_id {
private:
    std::string connection_id;

public:
    http_connection_id() = default;

    http_connection_id(std::shared_ptr<cppress::sockets::connection> conn) {
        connection_id =
            (conn->remote_endpoint().to_string() + ":" + std::to_string(conn->native_handle()));
    }
    http_connection_id operator=(std::shared_ptr<cppress::sockets::connection> conn) {
        connection_id =
            (conn->remote_endpoint().to_string() + ":" + std::to_string(conn->native_handle()));
        return *this;
    }

    std::string to_string() const { return connection_id; }
};
}  // namespace cppress::http