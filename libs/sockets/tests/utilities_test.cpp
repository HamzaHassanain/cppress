/**
 * @file test_utilities.cpp
 * @brief Unit tests for socket utility functions
 */

#include <gtest/gtest.h>

#include <cstring>

#include "includes/family.hpp"
#include "includes/ip_address.hpp"
#include "includes/port.hpp"
#include "includes/utilities.hpp"

using namespace cppress::sockets;

// ============================================================================
// Tests for convert_ip_address_to_network_order
// ============================================================================

TEST(UtilitiesTest, ConvertIPAddressToNetworkOrder_IPv4_Valid) {
    family fam = family::ipv4();
    ip_address addr("192.168.1.1");
    struct in_addr result;

    convert_ip_address_to_network_order(fam, addr, &result);

    // Convert back to string to verify
    char buffer[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &result, buffer, INET_ADDRSTRLEN);
    EXPECT_STREQ(buffer, "192.168.1.1");
}

TEST(UtilitiesTest, ConvertIPAddressToNetworkOrder_IPv4_Localhost) {
    family fam = family::ipv4();
    ip_address addr("127.0.0.1");
    struct in_addr result;

    convert_ip_address_to_network_order(fam, addr, &result);

    char buffer[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &result, buffer, INET_ADDRSTRLEN);
    EXPECT_STREQ(buffer, "127.0.0.1");
}

TEST(UtilitiesTest, ConvertIPAddressToNetworkOrder_IPv4_ZeroAddress) {
    family fam = family::ipv4();
    ip_address addr("0.0.0.0");
    struct in_addr result;

    convert_ip_address_to_network_order(fam, addr, &result);

    char buffer[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &result, buffer, INET_ADDRSTRLEN);
    EXPECT_STREQ(buffer, "0.0.0.0");
}

// ============================================================================
// Tests for get_ip_address_from_network_address
// ============================================================================

TEST(UtilitiesTest, GetIPAddressFromNetworkAddress_IPv4) {
    sockaddr_storage storage;
    std::memset(&storage, 0, sizeof(storage));

    sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(&storage);
    addr->sin_family = AF_INET;
    inet_pton(AF_INET, "10.0.0.1", &addr->sin_addr);

    std::string result = get_ip_address_from_network_address(storage);
    EXPECT_EQ(result, "10.0.0.1");
}

TEST(UtilitiesTest, GetIPAddressFromNetworkAddress_Localhost) {
    sockaddr_storage storage;
    std::memset(&storage, 0, sizeof(storage));

    sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(&storage);
    addr->sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &addr->sin_addr);

    std::string result = get_ip_address_from_network_address(storage);
    EXPECT_EQ(result, "127.0.0.1");
}

TEST(UtilitiesTest, GetIPAddressFromNetworkAddress_BroadcastAddress) {
    sockaddr_storage storage;
    std::memset(&storage, 0, sizeof(storage));

    sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(&storage);
    addr->sin_family = AF_INET;
    inet_pton(AF_INET, "255.255.255.255", &addr->sin_addr);

    std::string result = get_ip_address_from_network_address(storage);
    EXPECT_EQ(result, "255.255.255.255");
}

// ============================================================================
// Tests for is_valid_port
// ============================================================================

TEST(UtilitiesTest, IsValidPort_ValidPorts) {
    EXPECT_TRUE(is_valid_port(port(1024)));   // Minimum valid
    EXPECT_TRUE(is_valid_port(port(8080)));   // Common port
    EXPECT_TRUE(is_valid_port(port(65535)));  // Maximum valid
    EXPECT_TRUE(is_valid_port(port(3000)));
    EXPECT_TRUE(is_valid_port(port(50000)));
}

TEST(UtilitiesTest, IsValidPort_EdgeCases) {
    EXPECT_TRUE(is_valid_port(port(1024)));   // Minimum allowed
    EXPECT_TRUE(is_valid_port(port(65535)));  // Maximum allowed
}

TEST(UtilitiesTest, IsValidPort_CommonPorts) {
    EXPECT_TRUE(is_valid_port(port(8080)));  // HTTP alternate
    EXPECT_TRUE(is_valid_port(port(3000)));  // Node.js default
    EXPECT_TRUE(is_valid_port(port(5000)));  // Common dev port
}

// ============================================================================
// Tests for get_random_free_port
// ============================================================================

TEST(UtilitiesTest, GetRandomFreePort_ReturnsValidPort) {
    port p = get_random_free_port();
    EXPECT_TRUE(is_valid_port(p));
    EXPECT_GE(p.value(), MIN_PORT);
    EXPECT_LE(p.value(), MAX_PORT);
}

TEST(UtilitiesTest, GetRandomFreePort_ReturnsDifferentPorts) {
    port p1 = get_random_free_port();
    port p2 = get_random_free_port();

    // Ports should be valid
    EXPECT_TRUE(is_valid_port(p1));
    EXPECT_TRUE(is_valid_port(p2));

    // Note: They might be the same, but both should be valid
    EXPECT_GE(p1.value(), MIN_PORT);
    EXPECT_GE(p2.value(), MIN_PORT);
}

TEST(UtilitiesTest, GetRandomFreePort_MultipleCallsSucceed) {
    // Test that we can get multiple random ports
    for (int i = 0; i < 5; ++i) {
        port p = get_random_free_port();
        EXPECT_TRUE(is_valid_port(p));
    }
}

// ============================================================================
// Tests for convert_host_to_network_order
// ============================================================================

TEST(UtilitiesTest, ConvertHostToNetworkOrder_CommonPorts) {
    int port_8080 = convert_host_to_network_order(8080);
    EXPECT_EQ(convert_network_order_to_host(port_8080), 8080);

    int port_3000 = convert_host_to_network_order(3000);
    EXPECT_EQ(convert_network_order_to_host(port_3000), 3000);
}

TEST(UtilitiesTest, ConvertHostToNetworkOrder_EdgeCases) {
    int port_1024 = convert_host_to_network_order(1024);
    EXPECT_EQ(convert_network_order_to_host(port_1024), 1024);

    int port_65535 = convert_host_to_network_order(65535);
    EXPECT_EQ(convert_network_order_to_host(port_65535), 65535);
}

TEST(UtilitiesTest, ConvertHostToNetworkOrder_Roundtrip) {
    // Test roundtrip conversion
    int original = 12345;
    int network = convert_host_to_network_order(original);
    int back = convert_network_order_to_host(network);
    EXPECT_EQ(back, original);
}

// ============================================================================
// Tests for convert_network_order_to_host
// ============================================================================

TEST(UtilitiesTest, ConvertNetworkOrderToHost_CommonPorts) {
    // Create network order port using htons
    uint16_t network_8080 = htons(8080);
    int host_port = convert_network_order_to_host(network_8080);
    EXPECT_EQ(host_port, 8080);
}

TEST(UtilitiesTest, ConvertNetworkOrderToHost_EdgeCases) {
    uint16_t network_1024 = htons(1024);
    EXPECT_EQ(convert_network_order_to_host(network_1024), 1024);

    uint16_t network_65535 = htons(65535);
    EXPECT_EQ(convert_network_order_to_host(network_65535), 65535);
}

TEST(UtilitiesTest, ConvertNetworkOrderToHost_Roundtrip) {
    uint16_t network_port = htons(5555);
    int host = convert_network_order_to_host(network_port);
    int back_to_network = convert_host_to_network_order(host);
    EXPECT_EQ(back_to_network, network_port);
}

// ============================================================================
// Tests for initialize_socket_library / cleanup_socket_library
// ============================================================================

TEST(UtilitiesTest, InitializeSocketLibrary_Succeeds) {
    // Should succeed on all platforms
    EXPECT_TRUE(initialize_socket_library());
}

TEST(UtilitiesTest, InitializeSocketLibrary_MultipleCallsSucceed) {
    // Multiple initializations should be safe
    EXPECT_TRUE(initialize_socket_library());
    EXPECT_TRUE(initialize_socket_library());
}

TEST(UtilitiesTest, CleanupSocketLibrary_Succeeds) {
    // Should not crash or throw
    EXPECT_NO_THROW(cleanup_socket_library());
}

// ============================================================================
// Tests for is_valid_socket
// ============================================================================

TEST(UtilitiesTest, IsValidSocket_InvalidSocket) {
    EXPECT_FALSE(is_valid_socket(INVALID_SOCKET_VALUE));
}

TEST(UtilitiesTest, IsValidSocket_NegativeDescriptor) {
#ifndef SOCKET_PLATFORM_WINDOWS
    EXPECT_FALSE(is_valid_socket(-1));
    EXPECT_FALSE(is_valid_socket(-100));
#endif
}

TEST(UtilitiesTest, IsValidSocket_CreatedSocket) {
    // Create a real socket and test
    initialize_socket_library();
    socket_t sock = ::socket(AF_INET, SOCK_STREAM, 0);
    EXPECT_TRUE(is_valid_socket(sock));
    close_socket(sock);
}

// ============================================================================
// Tests for to_upper_case
// ============================================================================

TEST(UtilitiesTest, ToUpperCase_BasicString) {
    EXPECT_EQ(to_upper_case("hello"), "HELLO");
    EXPECT_EQ(to_upper_case("world"), "WORLD");
}

TEST(UtilitiesTest, ToUpperCase_MixedCase) {
    EXPECT_EQ(to_upper_case("HeLLo WoRLd"), "HELLO WORLD");
    EXPECT_EQ(to_upper_case("TeSt123"), "TEST123");
}

TEST(UtilitiesTest, ToUpperCase_SpecialCases) {
    EXPECT_EQ(to_upper_case(""), "");
    EXPECT_EQ(to_upper_case("123"), "123");
    EXPECT_EQ(to_upper_case("ALREADY_UPPER"), "ALREADY_UPPER");
    EXPECT_EQ(to_upper_case("with-dashes"), "WITH-DASHES");
}

// ============================================================================
// Tests for is_socket_open
// ============================================================================

TEST(UtilitiesTest, IsSocketOpen_InvalidDescriptor) {
    EXPECT_FALSE(is_socket_open(-1));
    EXPECT_FALSE(is_socket_open(INVALID_SOCKET_VALUE));
}

TEST(UtilitiesTest, IsSocketOpen_ValidSocket) {
    initialize_socket_library();
    socket_t sock = ::socket(AF_INET, SOCK_STREAM, 0);
    EXPECT_TRUE(is_socket_open(sock));
    close_socket(sock);
}

TEST(UtilitiesTest, IsSocketOpen_ClosedSocket) {
    initialize_socket_library();
    socket_t sock = ::socket(AF_INET, SOCK_STREAM, 0);
    close_socket(sock);
    // After closing, socket should not be open
    // Note: This may be platform-dependent
    EXPECT_FALSE(is_socket_open(sock));
}
