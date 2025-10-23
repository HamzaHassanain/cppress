#include "../includes/socket_address.hpp"

#include "../includes/family.hpp"
#include "../includes/ip_address.hpp"
#include "../includes/port.hpp"
#include "../includes/utilities.hpp"

namespace cppress::sockets {

socket_address::socket_address(const cppress::sockets::port& port_id, const ip_address& address,
                               const cppress::sockets::family& family_id)
    : address_(address), family_(family_id), port_(port_id) {
    handle_ipv4(this, address, port_id, family_id);
    handle_ipv6(this, address, port_id, family_id);
}

socket_address::socket_address(const cppress::sockets::ip_address& address,
                               const cppress::sockets::port& port_id,
                               const cppress::sockets::family& family_id)
    : address_(address), family_(family_id), port_(port_id) {
    handle_ipv4(this, address, port_id, family_id);
    handle_ipv6(this, address, port_id, family_id);
}

socket_address::socket_address(sockaddr_storage& addr) {
    // Check utility functions, to know about the internal implementations
    // If the IP family is IPv4
    if (addr.ss_family == IPV4) {
        auto ipv4_addr = reinterpret_cast<sockaddr_in*>(&addr);

        address_ = ip_address(get_ip_address_from_network_address(addr));
        port_ = cppress::sockets::port(convert_network_order_to_host(ipv4_addr->sin_port));
        family_ = cppress::sockets::family(IPV4);

        // Converts shared_ptr<sockaddr_in> to shared_ptr<sockaddr>
        // This is a safe cast because sockaddr_in can be safely treated as sockaddr
        this->addr =
            std::reinterpret_pointer_cast<sockaddr>(std::make_shared<sockaddr_in>(*ipv4_addr));
    } else if (addr.ss_family == IPV6) {
        auto ipv6_addr = reinterpret_cast<sockaddr_in6*>(&addr);
        address_ = ip_address(get_ip_address_from_network_address(addr));
        port_ = cppress::sockets::port(convert_network_order_to_host(ipv6_addr->sin6_port));
        family_ = cppress::sockets::family(IPV6);

        // Converts shared_ptr<sockaddr_in6> to shared_ptr<sockaddr>
        // This is a safe cast because sockaddr_in6 can be safely treated as sockaddr
        this->addr =
            std::reinterpret_pointer_cast<sockaddr>(std::make_shared<sockaddr_in6>(*ipv6_addr));
    }
}
socket_address::socket_address(const socket_address& other)
    : address_(other.address_), family_(other.family_), port_(other.port_) {
    if (other.addr) {
        handle_ipv4(this, other.address_, other.port_, other.family_);
        handle_ipv6(this, other.address_, other.port_, other.family_);
    }
}

socket_address& socket_address::operator=(const socket_address& other) {
    if (this != &other) {
        address_ = other.address_;
        family_ = other.family_;
        port_ = other.port_;

        if (other.addr) {
            handle_ipv4(this, other.address_, other.port_, other.family_);
            handle_ipv6(this, other.address_, other.port_, other.family_);
        }
    }
    return *this;
}

socklen_t socket_address::size() const {
    if (family_.value() == IPV4) {
        return sizeof(sockaddr_in);
    } else if (family_.value() == IPV6) {
        return sizeof(sockaddr_in6);
    }
    return 0;
}

sockaddr* socket_address::data() const {
    return addr.get();
}

void handle_ipv4(socket_address* addr, const ip_address& address,
                 const cppress::sockets::port& port_id, const cppress::sockets::family& family_id) {
    auto cur_addr = std::make_shared<sockaddr_in>();
    cur_addr->sin_family = family_id.value();
    cur_addr->sin_port = convert_host_to_network_order(port_id.value());
    convert_ip_address_to_network_order(family_id, address, &cur_addr->sin_addr);

    addr->addr = std::reinterpret_pointer_cast<sockaddr>(cur_addr);
}

void handle_ipv6(socket_address* addr, const ip_address& address,
                 const cppress::sockets::port& port_id, const cppress::sockets::family& family_id) {
    auto cur_addr = std::make_shared<sockaddr_in6>();
    cur_addr->sin6_family = family_id.value();
    cur_addr->sin6_port = convert_host_to_network_order(port_id.value());
    convert_ip_address_to_network_order(family_id, address, &cur_addr->sin6_addr);

    addr->addr = std::reinterpret_pointer_cast<sockaddr>(cur_addr);
}
}  // namespace cppress::sockets
