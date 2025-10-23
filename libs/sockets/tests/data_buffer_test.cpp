/**
 * @file test_data_buffer.cpp
 * @brief Unit tests for the data_buffer class
 */

#include <gtest/gtest.h>

#include <cstring>
#include <string>

#include "includes/data_buffer.hpp"

using namespace cppress::sockets;

/**
 * @test Test append operations and basic functionality
 * Tests appending strings, raw data, and other buffers
 */
TEST(DataBufferTest, AppendOperations) {
    data_buffer buf;
    EXPECT_TRUE(buf.empty());
    EXPECT_EQ(buf.size(), 0);

    // Append string
    buf.append("Hello");
    EXPECT_FALSE(buf.empty());
    EXPECT_EQ(buf.size(), 5);
    EXPECT_EQ(buf.to_string(), "Hello");

    // Append another string
    buf.append(" World");
    EXPECT_EQ(buf.size(), 11);
    EXPECT_EQ(buf.to_string(), "Hello World");

    // Append raw data
    const char* raw = "!";
    buf.append(raw, 1);
    EXPECT_EQ(buf.size(), 12);
    EXPECT_EQ(buf.to_string(), "Hello World!");

    // Append another buffer
    data_buffer buf2("!!!");
    buf.append(buf2);
    EXPECT_EQ(buf.size(), 15);
    EXPECT_EQ(buf.to_string(), "Hello World!!!!");

    // Test clear
    buf.clear();
    EXPECT_TRUE(buf.empty());
    EXPECT_EQ(buf.size(), 0);
}

/**
 * @test Test STL-like methods (data, size, empty, clear)
 * Verifies STL compatibility and proper memory management
 */
TEST(DataBufferTest, STLLikeMethods) {
    data_buffer buf("Test Data");

    // Test size
    EXPECT_EQ(buf.size(), 9);
    EXPECT_FALSE(buf.empty());

    // Test data() pointer
    const char* ptr = buf.data();
    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(std::strncmp(ptr, "Test Data", 9), 0);

    // Test copy construction
    data_buffer buf_copy(buf);
    EXPECT_EQ(buf_copy.size(), buf.size());
    EXPECT_EQ(buf_copy.to_string(), buf.to_string());

    // Test move construction
    data_buffer buf_move(std::move(buf_copy));
    EXPECT_EQ(buf_move.size(), 9);
    EXPECT_EQ(buf_move.to_string(), "Test Data");

    // Test copy assignment
    data_buffer buf3;
    buf3 = buf;
    EXPECT_EQ(buf3.to_string(), "Test Data");

    // Test move assignment
    data_buffer buf4;
    buf4 = std::move(buf_move);
    EXPECT_EQ(buf4.to_string(), "Test Data");

    // Test clear
    buf4.clear();
    EXPECT_TRUE(buf4.empty());
    EXPECT_EQ(buf4.size(), 0);
}

/**
 * @test Test binary data handling with null bytes
 * Ensures proper handling of binary data containing null bytes
 */
TEST(DataBufferTest, BinaryDataHandling) {
    // Create binary data with null bytes
    char binary_data[] = {0x01, 0x00, 0x02, 0x00, 0x03, 0x04, 0x00, 0x05};
    const size_t binary_size = 8;

    data_buffer buf(binary_data, binary_size);

    // Verify size is correct (not terminated at null byte)
    EXPECT_EQ(buf.size(), binary_size);

    // Verify data integrity
    const char* data_ptr = buf.data();
    for (size_t i = 0; i < binary_size; ++i) {
        EXPECT_EQ(data_ptr[i], binary_data[i]);
    }

    // Verify to_string preserves null bytes
    std::string str = buf.to_string();
    EXPECT_EQ(str.size(), binary_size);

    // Append more binary data
    char more_binary[] = {0x06, 0x00, 0x07};
    buf.append(more_binary, 3);
    EXPECT_EQ(buf.size(), binary_size + 3);

    // Verify appended data
    const char* full_data = buf.data();
    EXPECT_EQ(full_data[8], 0x06);
    EXPECT_EQ(full_data[9], 0x00);
    EXPECT_EQ(full_data[10], 0x07);

    // Test construction from string with null bytes
    std::string binary_str(binary_data, binary_size);
    data_buffer buf2(binary_str);
    EXPECT_EQ(buf2.size(), binary_size);

    // Compare buffers
    for (size_t i = 0; i < binary_size; ++i) {
        EXPECT_EQ(buf2.data()[i], binary_data[i]);
    }
}
