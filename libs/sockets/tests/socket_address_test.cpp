/**
 * @file test_socket_address.cpp
 * @brief Unit tests for the socket_address class
 */

#include <gtest/gtest.h>

#include "includes/family.hpp"
#include "includes/ip_address.hpp"
#include "includes/port.hpp"
#include "includes/socket_address.hpp"

using namespace cppress::sockets;

/**
 * @test Test socket_address construction and basic functionality
 * Verifies proper initialization and component storage
 */
TEST(SocketAddressTest, ConstructionAndBasicFunctionality) {
    // Test IPv4 construction
    ip_address ipv4_addr("192.168.1.100");
    port p(8080);
    family fam = family::ipv4();

    socket_address addr(ipv4_addr, p, fam);

    // Verify components are stored correctly
    EXPECT_EQ(addr.address().string(), "192.168.1.100");
    EXPECT_EQ(addr.port().value(), 8080);
    EXPECT_EQ(addr.family().value(), AF_INET);

    // Test with localhost
    socket_address localhost(ip_address("127.0.0.1"), port(3000), family::ipv4());
    EXPECT_EQ(localhost.address().string(), "127.0.0.1");
    EXPECT_EQ(localhost.port().value(), 3000);

    // Test with any address
    socket_address any_addr(ip_address("0.0.0.0"), port(5000), family::ipv4());
    EXPECT_EQ(any_addr.address().string(), "0.0.0.0");
    EXPECT_EQ(any_addr.port().value(), 5000);
}

/**
 * @test Test accessor methods (address, port, family)
 * Verifies all accessor methods return correct values
 */
TEST(SocketAddressTest, AccessorMethods) {
    socket_address addr(ip_address("10.0.0.50"), port(9000), family::ipv4());

    // Test address accessor
    auto retrieved_addr = addr.address();
    EXPECT_EQ(retrieved_addr.string(), "10.0.0.50");

    // Test port accessor
    auto retrieved_port = addr.port();
    EXPECT_EQ(retrieved_port.value(), 9000);

    // Test family accessor
    auto retrieved_family = addr.family();
    EXPECT_EQ(retrieved_family.value(), AF_INET);

    // Test copy construction
    socket_address addr_copy(addr);
    EXPECT_EQ(addr_copy.address().string(), "10.0.0.50");
    EXPECT_EQ(addr_copy.port().value(), 9000);
    EXPECT_EQ(addr_copy.family().value(), AF_INET);

    // Test assignment
    socket_address addr2(ip_address("1.1.1.1"), port(1024), family::ipv4());
    addr2 = addr;
    EXPECT_EQ(addr2.address().string(), "10.0.0.50");
    EXPECT_EQ(addr2.port().value(), 9000);
}

/**
 * @test Test data() and size() methods for raw sockaddr access
 * Verifies STL-style raw data access methods
 */
TEST(SocketAddressTest, RawDataAccess) {
    socket_address addr(ip_address("172.16.0.1"), port(4000), family::ipv4());

    // Test data() returns valid pointer
    const sockaddr* raw_addr = addr.data();
    EXPECT_NE(raw_addr, nullptr);

    // Test size() returns correct length
    socklen_t addr_len = addr.size();
    EXPECT_GT(addr_len, 0);
    EXPECT_EQ(addr_len, sizeof(sockaddr_in));  // IPv4 address

    // Verify the raw data is correct
    const sockaddr_in* ipv4_addr = reinterpret_cast<const sockaddr_in*>(raw_addr);
    EXPECT_EQ(ipv4_addr->sin_family, AF_INET);
    EXPECT_EQ(ntohs(ipv4_addr->sin_port), 4000);

    // Test with different addresses
    socket_address addr2(ip_address("192.168.0.1"), port(8888), family::ipv4());
    const sockaddr* raw_addr2 = addr2.data();
    socklen_t addr_len2 = addr2.size();

    EXPECT_NE(raw_addr2, nullptr);
    EXPECT_EQ(addr_len2, sizeof(sockaddr_in));

    const sockaddr_in* ipv4_addr2 = reinterpret_cast<const sockaddr_in*>(raw_addr2);
    EXPECT_EQ(ntohs(ipv4_addr2->sin_port), 8888);

    // Test that data() and size() can be used together (STL-style)
    // This simulates passing to bind() or connect()
    auto verify_addr = [](const sockaddr* addr, socklen_t len) {
        EXPECT_NE(addr, nullptr);
        EXPECT_GT(len, 0);
        return true;
    };

    EXPECT_TRUE(verify_addr(addr.data(), addr.size()));
    EXPECT_TRUE(verify_addr(addr2.data(), addr2.size()));
}
