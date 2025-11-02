#include <map>
#include <memory>
#include <string>

#include "../../sockets/includes.hpp"
#include "types.hpp"

namespace cppress::core {

class packet_router {
private:
    /// @brief  Routing table mapping packet keys to packet info, once we recive the headers,
    /// we know that this connection with fd maps to this packet info, when the http request is
    /// over, we remove this entry from the routing table (we do not close the connection, just
    /// remove the mapping)
    std::map<packet_key, packet_info> routing_table;

public:
    packet_router() = default;

    virtual void route_packet(std::shared_ptr<cppress::sockets::connection> conn,
                              cppress::sockets::data_buffer& packet);

    ~packet_router() = default;
};
}  // namespace cppress::core
