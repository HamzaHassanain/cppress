#include "../../sockets/includes.hpp"

namespace cppress::core {
namespace single_threaded {
class server : public cppress::sockets::epoll_server {};
}  // namespace single_threaded

class server {
    std::vector<single_threaded::server> servers;
};
}  // namespace cppress::core