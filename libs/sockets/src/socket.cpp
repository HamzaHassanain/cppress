#include <cstring>
#include <iostream>
#include <stdexcept>
#include <thread>
// Platform-specific includes
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#include <winsock2.h>
#include <ws2tcpip.h>
// #include <mstcpip.h>
#pragma comment(lib, "ws2_32.lib")
// Windows doesn't have errno for socket operations, use WSAGetLastError()
#define socket_errno() WSAGetLastError()
#define SOCKET_WOULDBLOCK WSAEWOULDBLOCK
#define SOCKET_AGAIN WSAEWOULDBLOCK
#else
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#define socket_errno() errno
#define SOCKET_WOULDBLOCK EWOULDBLOCK
#define SOCKET_AGAIN EAGAIN
#endif

#include "../includes/exceptions.hpp"
#include "../includes/file_descriptor.hpp"
#include "../includes/socket.hpp"
#include "../includes/utilities.hpp"

namespace cppress::sockets {

socket::socket(const socket::type& socket_type = socket::type::datagram)
    : socket_type(socket_type) {
    int socket_file_descriptor = ::socket(AF_INET, static_cast<int>(socket_type), 0);

    if (!is_valid_socket(socket_file_descriptor)) {
        throw socket_exception("Invalid File Descriptor", "SocketCreation", __func__);
    }

    fd = file_descriptor(socket_file_descriptor);
}

socket::socket(const socket_address& addr, const socket::type& socket_type) : addr(addr) {
    int socket_file_descriptor = ::socket(addr.family().value(), static_cast<int>(socket_type), 0);

    if (!is_valid_socket(socket_file_descriptor)) {
        throw socket_exception("Invalid File Descriptor", "SocketCreation", __func__);
    }

    fd = file_descriptor(socket_file_descriptor);
    this->socket_type = socket_type;
    this->bind(addr);
}

socket::socket(const family& fam, const socket::type& socket_type) : socket_type(socket_type) {
    int socket_file_descriptor = ::socket(fam.value(), static_cast<int>(socket_type), 0);

    if (!is_valid_socket(socket_file_descriptor)) {
        throw socket_exception("Invalid File Descriptor", "SocketCreation", __func__);
    }

    fd = file_descriptor(socket_file_descriptor);
    this->socket_type = socket_type;
}

std::shared_ptr<connection> socket::connect(const socket_address& server_address,
                                            bool NON_BLOCKING) {
    if (::connect(fd.native_handle(), server_address.data(), server_address.size()) ==
        SOCKET_ERROR_VALUE) {
        throw socket_exception("Failed to connect to address: " + std::string(get_error_message()),
                               "SocketConnection", __func__);
    }

    if (NON_BLOCKING) {
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
        u_long mode = 1;  // 1 to enable non-blocking socket
        if (ioctlsocket(fd.native_handle(), FIONBIO, &mode) != 0) {
            throw socket_exception(
                "Failed to set non-blocking mode: " + std::string(get_error_message()),
                "SocketOption", __func__);
        }
#else
        int flags = fcntl(fd.native_handle(), F_GETFL, 0);
        if (flags == -1) {
            throw socket_exception(
                "Failed to get socket flags: " + std::string(get_error_message()), "SocketOption",
                __func__);
        } else {
            if (fcntl(fd.native_handle(), F_SETFL, flags | O_NONBLOCK) == -1) {
                throw socket_exception(
                    "Failed to set non-blocking mode: " + std::string(get_error_message()),
                    "SocketOption", __func__);
            }
        }
#endif
    }
    return std::make_shared<connection>(std::move(fd), this->get_bound_address(), server_address);
}
void socket::bind(const socket_address& addr) {
    this->addr = addr;

    if (::bind(fd.native_handle(), this->addr.data(), this->addr.size()) == SOCKET_ERROR_VALUE) {
        throw socket_exception("Failed to bind to address: " + std::string(get_error_message()),
                               "SocketBinding", __func__);
    }
}

void socket::listen(int backlog) {
    if (socket_type != type::stream) {
        throw socket_exception("Listen is only supported for TCP sockets", "socket::typeMismatch",
                               __func__);
    }

    if (::listen(fd.native_handle(), backlog) == SOCKET_ERROR_VALUE) {
        throw socket_exception("Failed to listen on socket: " + std::string(get_error_message()),
                               "SocketListening", __func__);
    }
}

std::shared_ptr<connection> socket::accept(bool NON_BLOCKING) {
    if (socket_type != type::stream) {
        throw socket_exception("Accept is only supported for TCP sockets", "socket::typeMismatch",
                               __func__);
    }
    if (fd.native_handle() == SOCKET_ERROR_VALUE) {
        throw socket_exception("Socket is not open", "SocketAcceptance", __func__);
    }
    sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    socket_t client_fd;
    if (!NON_BLOCKING) {
        client_fd = ::accept(fd.native_handle(), reinterpret_cast<sockaddr*>(&client_addr),
                             &client_addr_len);
        if (client_fd == INVALID_SOCKET_VALUE) {
            throw socket_exception(
                "Failed to accept connection: " + std::string(get_error_message()),
                "SocketAcceptance", __func__);
        }
    } else {
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
        // use non-blocking accept on Windows
        client_fd = ::accept(fd.native_handle(), reinterpret_cast<sockaddr*>(&client_addr),
                             &client_addr_len);
        if (client_fd != INVALID_SOCKET) {
            u_long mode = 1;  // 1 to enable non-blocking socket
            if (ioctlsocket(client_fd, FIONBIO, &mode) != 0) {
                closesocket(client_fd);
                client_fd = INVALID_SOCKET;
                throw socket_exception("Failed to set non-blocking mode on accepted socket: " +
                                           std::string(get_error_message()),
                                       "SocketOption", __func__);
            }
        }
#else
        // Use non-blocking accept on UNIX
        client_fd = ::accept4(fd.native_handle(), reinterpret_cast<sockaddr*>(&client_addr),
                              &client_addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
    }
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    int error = WSAGetLastError();
    if (error == WSAEWOULDBLOCK)
#else
    if (errno == EAGAIN || errno == EWOULDBLOCK)
#endif
    {
        // no connection to accept
        return std::shared_ptr<connection>(nullptr);
    }

    if (!is_valid_socket(client_fd)) {
        throw socket_exception("Failed to accept connection: " + std::string(get_error_message()),
                               "SocketAcceptance", __func__);
    }

    return std::make_shared<connection>(file_descriptor(client_fd), this->get_bound_address(),
                                        socket_address(client_addr));
}

data_buffer socket::receive(socket_address& client_addr) {
    if (socket_type != type::datagram) {
        throw socket_exception("receive is only supported for UDP sockets", "socket::typeMismatch",
                               __func__);
    }

    sockaddr_storage sender_addr;
    socklen_t sender_addr_len = sizeof(sender_addr);

    // Use 64KB buffer for UDP - theoretical max UDP payload is 65507 bytes
    char buffer[MAX_BUFFER_SIZE];

    int bytes_received = ::recvfrom(fd.native_handle(), buffer, sizeof(buffer), 0,
                                    reinterpret_cast<sockaddr*>(&sender_addr), &sender_addr_len);

    if (bytes_received == SOCKET_ERROR_VALUE) {
        throw socket_exception("Failed to receive data: " + std::string(get_error_message()),
                               "SocketReceive", __func__);
    }

    client_addr = socket_address(sender_addr);
    return data_buffer(buffer, static_cast<std::size_t>(bytes_received));
}

void socket::send_to(const socket_address& addr, const data_buffer& data) {
    // Verify this is a UDP socket - TCP uses send_on_connection()
    if (socket_type != type::datagram) {
        throw socket_exception("send_to is only supported for UDP sockets", "socket::typeMismatch",
                               __func__);
    }

    int bytes_sent =
        ::sendto(fd.native_handle(), data.data(), data.size(), 0, addr.data(), addr.size());

    if (bytes_sent == SOCKET_ERROR_VALUE) {
        throw socket_exception("Failed to send data: " + std::string(get_error_message()),
                               "SocketSend", __func__);
    }

    if (static_cast<std::size_t>(bytes_sent) != data.size()) {
        throw socket_exception("Partial send: only " + std::to_string(bytes_sent) + " of " +
                                   std::to_string(data.size()) + " bytes sent",
                               "PartialSend", __func__);
    }
}

socket_address socket::get_bound_address() const {
    return addr;
}

int socket::native_handle() const {
    return fd.native_handle();
}

int socket::get_fd() const {
    return native_handle();
}

void socket::set_close_on_exec(bool enable) {
    try {
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
        HANDLE handle = reinterpret_cast<HANDLE>(this->fd.native_handle());
        DWORD flags = 0;

        if (GetHandleInformation(handle, &flags)) {
            if (enable)
                SetHandleInformation(handle, HANDLE_FLAG_INHERIT, 0);  // Disable inheritance
            else
                SetHandleInformation(handle, HANDLE_FLAG_INHERIT,
                                     HANDLE_FLAG_INHERIT);  // Enable inheritance
        } else {
            throw std::runtime_error("Failed to get handle information");
        }
#else

        int flags = fcntl(this->fd.native_handle(), F_GETFD);
        if (flags != -1) {
            if (enable)
                fcntl(this->fd.native_handle(), F_SETFD, flags | FD_CLOEXEC);
            else
                fcntl(this->fd.native_handle(), F_SETFD, flags & ~FD_CLOEXEC);
        }
#endif
    } catch (const std::exception& e) {
        throw socket_exception("Error setting close-on-exec flag: " + std::string(e.what()),
                               "SocketSetCloseOnExec", __func__);
    }
}

void socket::set_reuse_address(bool reuse) {
    int optval = reuse ? 1 : 0;

    const char* optval_ptr = reinterpret_cast<const char*>(&optval);
    if (setsockopt(fd.native_handle(), SOL_SOCKET, SO_REUSEADDR, optval_ptr, sizeof(optval)) ==
        SOCKET_ERROR_VALUE) {
        throw socket_exception(
            "Failed to set SO_REUSEADDR option: " + std::string(get_error_message()),
            "SocketOption", __func__);
    }
}

void socket::set_reuse_port(bool reuse) {
    int optval = reuse ? 1 : 0;

    const char* optval_ptr = reinterpret_cast<const char*>(&optval);
    if (setsockopt(fd.native_handle(), SOL_SOCKET, SO_REUSEPORT, optval_ptr, sizeof(optval)) ==
        SOCKET_ERROR_VALUE) {
        throw socket_exception(
            "Failed to set SO_REUSEPORT option: " + std::string(get_error_message()),
            "SocketOption", __func__);
    }
}

void socket::set_non_blocking(bool enable) {
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    // Windows implementation using ioctlsocket
    u_long mode = enable ? 1 : 0;
    if (ioctlsocket(fd.native_handle(), FIONBIO, &mode) != 0) {
        throw socket_exception(
            "Failed to set socket non-blocking mode: " + std::string(get_error_message()),
            "SocketOption", __func__);
    }
#else
    // UNIX/Linux implementation using fcntl
    // Get current file descriptor flags
    int flags = fcntl(fd.native_handle(), F_GETFL, 0);
    if (flags == -1) {
        throw socket_exception("Failed to get socket flags: " + std::string(get_error_message()),
                               "SocketOption", __func__);
    }

    // Modify O_NONBLOCK flag based on enable parameter
    if (enable) {
        flags |= O_NONBLOCK;  // Set non-blocking
    } else {
        flags &= ~O_NONBLOCK;  // Clear non-blocking (set blocking)
    }

    // Apply modified flags back to file descriptor
    if (fcntl(fd.native_handle(), F_SETFL, flags) == -1) {
        throw socket_exception(
            "Failed to set socket non-blocking mode: " + std::string(get_error_message()),
            "SocketOption", __func__);
    }
#endif
}

void socket::set_option(int level, int optname, int optval) {
    // setsockopt(sockfd, level, optname, optval, optlen) - set socket option
    // Returns 0 on success, -1 on error
    const char* optval_ptr = reinterpret_cast<const char*>(&optval);
    if (setsockopt(fd.native_handle(), level, optname, optval_ptr, sizeof(optval)) ==
        SOCKET_ERROR_VALUE) {
        throw socket_exception("Failed to set socket option: " + std::string(get_error_message()),
                               "SocketOption", __func__);
    }
}

bool socket::is_open() const {
    return open_;
}

bool socket::is_connected() const {
    return is_open();
}

void socket::close() {
    if (open_) {
        // Close the socket and mark it as closed
        close_socket(fd.native_handle());
        fd.invalidate();
        open_ = false;
    }
}

void socket::disconnect() {
    close();
}

socket::~socket() {
    close();
}
}  // namespace cppress::sockets