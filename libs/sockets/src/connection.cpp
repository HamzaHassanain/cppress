#include "../includes/connection.hpp"

#include "../includes/socket.hpp"
#include "../includes/utilities.hpp"
namespace cppress::sockets {

connection::connection(file_descriptor fd, const socket_address& local_addr,
                       const socket_address& remote_addr)
    : fd(std::move(fd)), local_addr(local_addr), remote_addr(remote_addr), open_(true) {
    if (this->fd.native_handle() == INVALID_SOCKET_VALUE ||
        this->fd.native_handle() == SOCKET_ERROR_VALUE) {
        throw socket_exception("Invalid file descriptor", "ConnectionCreation", __func__);
    }
}

connection::connection(const socket_address& remote_addr) : remote_addr(remote_addr), open_(true) {
    this->connect(remote_addr);
}

std::size_t connection::write(const data_buffer& data) {
    if (!open_ || fd.native_handle() == SOCKET_ERROR_VALUE ||
        fd.native_handle() == INVALID_SOCKET_VALUE) {
        return 0;
    }
    auto bytes_sent = ::send(fd.native_handle(), data.data(), data.size(), 0);
    if (bytes_sent == SOCKET_ERROR_VALUE) {
        throw socket_exception(
            "Failed to write data for fd:  " + std::to_string(fd.native_handle()) + " " +
                std::string(get_error_message()),
            "SocketWrite", __func__);
    }
    return bytes_sent;
}

std::size_t connection::send(const data_buffer& data) {
    return write(data);
}

data_buffer connection::read() {
    if (!open_ || fd.native_handle() == SOCKET_ERROR_VALUE ||
        fd.native_handle() == INVALID_SOCKET_VALUE) {
        return data_buffer();
    }

    data_buffer received_data;
    char buffer[MAX_BUFFER_SIZE];

    int bytes_received = ::recv(fd.native_handle(), buffer, sizeof(buffer), 0);

    /// EOF
    if (bytes_received == 0) {
        return data_buffer();
    }
    if (bytes_received == SOCKET_ERROR_VALUE) {
        /*
        EAGAIN or EWOULDBLOCK: The socket is non-blocking, and no data is currently available.
        ENOTCONN: The socket is not connected.

        @throw error for the follwing return values
        ECONNRESET: The connection was forcibly closed by the peer.
        EINTR: The function call was interrupted by a signal.
        */
#if defined(SOCKET_PLATFORM_UNIX)
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
            return data_buffer();
        }
#elif defined(SOCKET_PLATFORM_WINDOWS)
        if (WSAGetLastError() == WSAEWOULDBLOCK || WSAGetLastError() == WSAEINTR) {
            return data_buffer();
        }
#endif
        throw socket_exception("Failed to read data for fd " + std::to_string(fd.native_handle()) +
                                   " " + std::string(get_error_message()),
                               "SocketRead", __func__);
    }

    received_data.append(buffer, bytes_received);
    return received_data;
}

data_buffer connection::receive() {
    return read();
}

void connection::close() {
    if (open_) {
        open_ = false;
        close_socket(fd.native_handle());
        fd.invalidate();
    }
}

bool connection::is_open() const {
    return open_;
}

bool connection::is_connection_open() const {
    return is_open();
}

connection::~connection() {
    close();
}

void connection::connect(const socket_address& remote_addr) {
    this->remote_addr = remote_addr;

    int socket_file_descriptor =
        ::socket(remote_addr.family().value(), static_cast<int>(socket::type::stream), 0);

    if (!is_valid_socket(socket_file_descriptor)) {
        throw socket_exception("Invalid File Descriptor", "ConnectionCreation", __func__);
    }

    fd = file_descriptor(socket_file_descriptor);

    if (::connect(fd.native_handle(), remote_addr.data(), remote_addr.size()) ==
        SOCKET_ERROR_VALUE) {
        throw socket_exception("Failed to connect to address: " + std::string(get_error_message()),
                               "SocketConnection", __func__);
    }

    sockaddr_storage local_addr_storage;
    socklen_t addr_len = sizeof(local_addr_storage);
    if (getsockname(fd.native_handle(), reinterpret_cast<sockaddr*>(&local_addr_storage),
                    &addr_len) == SOCKET_ERROR_VALUE) {
        throw socket_exception("Failed to get local address: " + std::string(get_error_message()),
                               "SocketGetLocalAddress", __func__);
    }
    local_addr = socket_address(local_addr_storage);
}

};  // namespace cppress::sockets
