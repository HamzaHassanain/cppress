/**
 * @file data_buffer.hpp
 * @brief Dynamic buffer for binary data management in the cppress sockets library.
 *
 * This file provides the data_buffer class, a STL-compliant container for storing
 * and managing binary data in network and file I/O operations. It wraps std::vector<char>
 * with a convenient interface designed for accumulating data from multiple sources.
 *
 * @section usage Common Usage Patterns
 *
 * **Basic Construction and Move Semantics:**
 * @code
 * #include "data_buffer.hpp"
 * using namespace cppress::sockets;
 *
 * // Create empty buffer
 * data_buffer buf1;
 *
 * // Create from string
 * data_buffer buf2("Hello, World!");
 *
 * // Create from raw data
 * char raw[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F};
 * data_buffer buf3(raw, 5);
 *
 * // Move-only: cannot copy, must use std::move
 * // data_buffer buf4 = buf2;  // ERROR: Copy constructor deleted
 * data_buffer buf4 = std::move(buf2);  // OK: Move constructor
 * // buf2 is now empty after the move
 * @endcode
 *
 * **Network I/O Usage:**
 * @code
 * // Accumulate data from network socket
 * data_buffer received_data;
 * connection conn = server.accept();
 *
 * while (conn.is_open()) {
 *     auto chunk = conn.read();  // Returns data_buffer (move semantics)
 *     received_data.append(chunk);
 *
 *     if (received_data.size() >= expected_size) {
 *         break;
 *     }
 * }
 *
 * // Process complete data
 * std::string message = received_data.to_string();
 *
 * // Transfer ownership using move semantics
 * data_buffer transferred = std::move(received_data);
 * // received_data is now empty
 * @endcode
 *
 * **STL-like Operations:**
 * @code
 * data_buffer buf;
 * buf.append("Test data");
 *
 * // Query buffer state
 * if (!buf.empty()) {
 *     std::size_t bytes = buf.size();
 *     const char* raw_ptr = buf.data();
 *
 *     // Send over network
 *     conn.write(buf);
 * }
 *
 * // Clear when done
 * buf.clear();
 * @endcode
 *
 * @section integration Integration with Sockets Library
 * The data_buffer class is designed to work seamlessly with other cppress::sockets
 * components:
 * - connection::read() returns data_buffer
 * - connection::write() accepts data_buffer
 * - Efficient for HTTP request/response body handling
 * - Used in streaming protocols for data accumulation
 *
 * @section performance Performance Characteristics
 * - Move-only class: Copy operations are deleted to prevent accidental copying
 * - Move operations: O(1) constant time
 * - Memory: Single contiguous allocation via std::vector
 * - Clear operation: Deallocates memory with shrink_to_fit()
 *
 * @author Hamza Moahmmed Hassanain
 * @version 1.0
 */

#pragma once

#include <string>
#include <vector>

#include "utilities.hpp"

namespace cppress::sockets {
/**
 * @brief A dynamic buffer for storing and managing binary data.
 *
 * This class provides a convenient wrapper around std::vector<char> for handling
 * binary data, strings, and character arrays. It offers efficient memory management
 * with automatic resizing and supports both string and raw data operations.
 *
 * The class is designed for scenarios where you need to accumulate data from
 * multiple sources (like network I/O, file reading, or HTTP parsing) and provides
 * seamless conversion between different data representations.
 *
 * @note Move-only type: Copy operations are explicitly deleted to prevent accidental
 * copying of potentially large buffers. Use move semantics (std::move) to transfer
 * ownership efficiently.
 * @note Uses explicit constructors to prevent implicit conversions for type safety.
 */
class data_buffer {
private:
    /// Internal storage for the buffer data
    std::vector<char> buffer;

public:
    /**
     * @brief Default constructor - creates an empty buffer.
     *
     * Creates a data_buffer with no initial data. The buffer will be empty
     * and ready to accept data through append operations.
     * Marked explicit to prevent implicit conversions.
     */
    data_buffer() = default;

    /**
     * @brief Construct buffer from a string.
     * @param str String to initialize the buffer with
     *
     * Creates a data_buffer containing a copy of the string's characters.
     * The resulting buffer will have the same content as the string.
     */
    data_buffer(const std::string& str) {
        if (str.size() < buffer.max_size()) {
            buffer.insert(buffer.end(), str.begin(), str.end());
        }
    }

    /**
     * @brief Construct buffer from raw character data.
     * @param data Pointer to the character data to copy
     * @param size Number of bytes to copy from data
     *
     * Creates a data_buffer containing a copy of the specified number of bytes
     * from the provided character array. This is useful for binary data that
     * may contain null bytes.
     *
     * @warning The caller must ensure that 'data' points to at least 'size' bytes
     *
     * Example:
     * @code
     * char raw_data[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x00, 0x57};
     * data_buffer buf(raw_data, 7);  // Includes the null byte
     * @endcode
     */
    data_buffer(const char* data, std::size_t size) {
        if (size < buffer.max_size()) {
            buffer.insert(buffer.end(), data, data + size);
        }
    }

    /**
     * @brief Construct buffer from null-terminated C-string.
     * @param data Null-terminated C-string to initialize the buffer with
     *   @note if data is nullptr or not null-terminated, behavior is undefined.
     */
    data_buffer(const char* data) {
        std::size_t size = std::strlen(data);
        if (size < buffer.max_size()) {
            buffer.insert(buffer.end(), data, data + size);
        }
    }

    // Move-only: copy operations deleted to prevent accidental copying of potentially large buffers
    /// @brief Copy constructor (deleted) - class is move-only.
    data_buffer(const data_buffer& other) = delete;

    /// @brief Copy assignment operator (deleted) - class is move-only.
    data_buffer& operator=(const data_buffer& other) = delete;

    // Move operations
    /**
     * @brief Move constructor.
     * @param other Buffer to move from
     *
     * Efficiently transfers ownership of the buffer data from another data_buffer.
     * The source buffer becomes empty after the move. This operation is O(1)
     * and doesn't copy the actual data.
     */
    data_buffer(data_buffer&& other) {
        buffer = std::move(other.buffer);
        other.clear();
    }

    /**
     * @brief Move assignment operator.
     * @param other Buffer to move from
     * @return Reference to this buffer after assignment
     */
    data_buffer& operator=(data_buffer&& other) {
        if (this != &other) {
            buffer = std::move(other.buffer);
            other.clear();
        }
        return *this;
    }

    /**
     * @brief Get a pointer to the buffer's data.
     * @return Const pointer to the first byte of the buffer
     *
     * Returns a pointer to the internal character array. The pointer is valid
     * until the next non-const operation on the buffer. For empty buffers,
     * the returned pointer may or may not be null, but should not be dereferenced.
     */
    const char* data() const { return buffer.data(); }

    /**
     * @brief Get the size of the buffer in bytes.
     * @return Number of bytes currently stored in the buffer
     *
     * Returns the total number of bytes contained in the buffer.
     * For empty buffers, this returns 0.
     */
    std::size_t size() const { return buffer.size(); }

    /**
     * @brief Check if the buffer is empty.
     * @return true if the buffer contains no data, false otherwise
     *
     * This is equivalent to checking if size() == 0, but may be more efficient.
     */
    bool empty() const { return buffer.empty(); }

    /**
     * @brief Clear all data from the buffer.
     *
     * Removes all data from the buffer, making it empty. size() will return 0 after this call.
     */
    void clear() {
        buffer.clear();
        buffer.shrink_to_fit();
    }

    /**
     * @brief Convert the buffer contents to a string.
     * @return String containing a copy of the buffer's data
     *
     * Creates a new std::string containing all the bytes from the buffer.
     * This operation creates a copy of the data, so the original buffer
     * remains unchanged.
     *
     * @note If the buffer contains null bytes, they will be included in the string
     */
    std::string to_string() const { return std::string(buffer.begin(), buffer.end()); }

    /// Default destructor
    ~data_buffer() = default;
};
}  // namespace cppress::sockets