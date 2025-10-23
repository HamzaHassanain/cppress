## 1. ❌ HTTP/2 or HTTP/3 Protocol Support

**Complexity**: Very High ⭐⭐⭐⭐⭐

**Explanation**:

- HTTP/2 uses binary framing instead of text-based protocol
- Requires complete rewrite of parsing logic
- Supports multiplexing (multiple requests on single connection)
- Uses HPACK for header compression
- Requires stream management and flow control

**Changes Needed**:

- New `http2_frame_parser.hpp/cpp` to handle binary frames
- New `http2_stream.hpp/cpp` for stream management
- HPACK compression/decompression library integration
- Flow control implementation
- Server push support
- Completely separate from HTTP/1.1 (different code path)

**Why it's hard**: Complete protocol redesign, not just an enhancement.

---

## 2. ❌ Chunked Transfer Encoding

**Complexity**: Medium ⭐⭐⭐

**Explanation**:
Chunked encoding allows sending body data without knowing total size upfront. Format:

```
[chunk size in hex]\r\n
[chunk data]\r\n
[next chunk size]\r\n
[next chunk data]\r\n
0\r\n
\r\n
```

**Changes Needed**:

**In `http_message_handler.hpp/cpp`**:

```cpp
// Add new handling type (already exists but not implemented)
enum class handling_type {
    CONTENT_LENGTH,
    CHUNKED,  // <-- Implement this
    UNKNOWN
};

// New private method in http_message_handler
http_handled_data handle_chunked_encoding(
    const std::string& socket_key,
    std::istringstream& request_stream,
    const std::string& method,
    const std::string& uri,
    const std::string& version,
    const std::multimap<std::string, std::string>& headers,
    int FD
);

// Parser for chunk format
std::pair<bool, size_t> parse_chunk_size(const std::string& line);
```

**In http_data_under_handling.hpp**:

```cpp
struct http_data_under_handling {
    // Add fields for chunked state
    bool waiting_for_chunk_size;
    size_t current_chunk_remaining;
    bool reading_trailer_headers;
};
```

**In `start_handling()` method**: Check for `Transfer-Encoding: chunked` header and call `handle_chunked_encoding()` instead of `handle_content_length()`.

**Implementation Steps**:

1. Parse hex chunk size
2. Read exactly that many bytes
3. Expect `\r\n` after chunk data
4. Repeat until chunk size is 0
5. Optionally parse trailer headers
6. Expect final `\r\n`

---

## 3. ❌ Automatic Compression (gzip, brotli)

**Complexity**: Medium ⭐⭐⭐

**Explanation**:
Compress response bodies based on `Accept-Encoding` header from client.

**Changes Needed**:

**New files**:

- `http_compression.hpp/cpp`

```cpp
namespace cppress::http {
    enum class compression_type {
        NONE,
        GZIP,
        DEFLATE,
        BROTLI
    };

    class http_compressor {
    public:
        static compression_type negotiate_encoding(
            const std::vector<std::string>& accept_encoding
        );

        static std::string compress_gzip(const std::string& data);
        static std::string compress_brotli(const std::string& data);
        static std::string compress_deflate(const std::string& data);
    };
}
```

**In `http_response.cpp`**:

```cpp
void http_response::send() {
    // Check if compression should be applied
    auto accept_encoding = /* get from request headers */;
    auto encoding = http_compressor::negotiate_encoding(accept_encoding);

    if (encoding != compression_type::NONE && body.size() > MIN_COMPRESSION_SIZE) {
        body = http_compressor::compress_gzip(body);
        add_header("Content-Encoding", "gzip");
    }

    // Update Content-Length after compression
    clear_header_values("Content-Length");
    add_header("Content-Length", std::to_string(body.size()));

    // Send...
}
```

**Dependencies**:

- zlib library for gzip/deflate
- brotli library for brotli compression

**CMakeLists.txt changes**:

```cmake
find_package(ZLIB REQUIRED)
find_package(BrotliEnc REQUIRED)
target_link_libraries(http PUBLIC ZLIB::ZLIB BrotliEnc::BrotliEnc)
```

---

## 4. ❌ Keep-alive / Persistent Connections

**Complexity**: High ⭐⭐⭐⭐

**Explanation**:
Allow multiple requests over same TCP connection instead of closing after each response.

**Changes Needed**:

**In http_server.hpp**:

```cpp
private:
    // Track connection states
    std::map<int, connection_state> active_connections;

    struct connection_state {
        int request_count;
        std::chrono::steady_clock::time_point last_activity;
        bool keep_alive_requested;
    };
```

**In `http_response.cpp`**:

```cpp
void http_response::send() {
    // Check Connection header from request
    auto conn_header = request_headers.get_header("Connection");

    if (/* should keep alive */) {
        add_header("Connection", "keep-alive");
        add_header("Keep-Alive", "timeout=5, max=100");
        // DON'T call close_connection after send
    } else {
        add_header("Connection", "close");
    }

    // Send response...
}

void http_response::end() {
    // Only close if Connection: close
    if (should_close_connection()) {
        close_connection();
    }
}
```

**In `http_server.cpp` - `on_message_received()`**:

```cpp
void http_server::on_message_received(...) {
    // Parse request...

    // Handle request...

    // DON'T automatically close connection
    // Let response.end() decide based on Connection header

    // If keep-alive, wait for next request on same connection
    if (keep_alive) {
        // Connection stays in epoll, ready for next request
        return;
    }
}
```

**Challenges**:

- Request pipelining (multiple requests without waiting for responses)
- Connection timeout management
- Max requests per connection limit
- Proper cleanup on errors

---

## 5. ❌ Range Requests / Partial Content

**Complexity**: Medium ⭐⭐⭐

**Explanation**:
Support `Range: bytes=0-1023` header for partial content (HTTP 206 responses).

**Changes Needed**:

**New parsing logic**:

```cpp
struct range_spec {
    std::optional<size_t> start;
    std::optional<size_t> end;
    bool valid;
};

range_spec parse_range_header(const std::string& range_value, size_t total_size);
```

**In request handler**:

```cpp
void handle_request(http_request& req, http_response& res) {
    auto range_header = req.get_header("Range");

    if (!range_header.empty()) {
        auto range = parse_range_header(range_header[0], file_size);

        if (range.valid) {
            res.set_status(206, "Partial Content");
            res.add_header("Content-Range",
                "bytes " + std::to_string(range.start) + "-" +
                std::to_string(range.end) + "/" +
                std::to_string(file_size));
            res.set_body(get_partial_content(range.start, range.end));
        }
    } else {
        res.set_status(200, "OK");
        res.set_body(get_full_content());
    }
}
```

**Additional Headers**:

- `Accept-Ranges: bytes` (in response)
- `Content-Range: bytes 0-1023/2048` (in 206 response)
- Support for multiple ranges (multipart/byteranges)

---

## 6. ❌ Multipart Form-Data Parsing

**Complexity**: Medium-High ⭐⭐⭐⭐

**Explanation**:
Parse `multipart/form-data` request bodies (file uploads).

**Format**:

```
------WebKitFormBoundary7MA4YWxkTrZu0gW
Content-Disposition: form-data; name="file"; filename="test.txt"
Content-Type: text/plain

[file contents here]
------WebKitFormBoundary7MA4YWxkTrZu0gW--
```

**Changes Needed**:

**New files**: `http_multipart_parser.hpp/cpp`

```cpp
struct multipart_part {
    std::string name;
    std::string filename;
    std::string content_type;
    std::string data;
    std::map<std::string, std::string> headers;
};

class multipart_parser {
public:
    static std::vector<multipart_part> parse(
        const std::string& body,
        const std::string& boundary
    );

private:
    static std::string extract_boundary(const std::string& content_type);
    static multipart_part parse_part(const std::string& part_data);
};
```

**Usage**:

```cpp
void handle_upload(http_request& req, http_response& res) {
    auto ct = req.get_header("Content-Type")[0];

    if (ct.find("multipart/form-data") != std::string::npos) {
        auto parts = multipart_parser::parse(req.get_body(), ct);

        for (auto& part : parts) {
            if (!part.filename.empty()) {
                save_file(part.filename, part.data);
            }
        }
    }
}
```

**Challenges**:

- Boundary extraction
- Handling large files (streaming needed for efficiency)
- Binary data handling
- Nested multipart (rare but exists)

---

## 7. ❌ SSL/TLS Support

**Complexity**: High ⭐⭐⭐⭐

**Explanation**:
Encrypt HTTP traffic using SSL/TLS (HTTPS).

**Changes Needed**:

**In `sockets` layer** (not HTTP layer):

```cpp
class ssl_socket : public socket {
private:
    SSL* ssl_handle;
    SSL_CTX* ssl_context;

public:
    ssl_socket(const socket_address& addr, const ssl_config& config);

    ssize_t send_ssl(const void* data, size_t size) override;
    ssize_t receive_ssl(void* buffer, size_t size) override;
};
```

**Configuration**:

```cpp
struct ssl_config {
    std::string certificate_path;
    std::string private_key_path;
    std::string ca_bundle_path;
    ssl_protocol min_protocol = TLS_1_2;
};
```

**In http_server.hpp**:

```cpp
class https_server : public http_server {
public:
    https_server(int port, const ssl_config& config);
};
```

**Dependencies**:

- OpenSSL or mbedTLS library

**Why use reverse proxy instead**:

- SSL/TLS is complex (certificate management, cipher suites, etc.)
- nginx/Apache are battle-tested
- Easier to update security patches
- Better performance (nginx SSL is optimized)
- Can handle multiple domains/certificates

---

## Summary of Difficulty and Priority:

| Feature                   | Complexity | Implementation Time | Priority                   |
| ------------------------- | ---------- | ------------------- | -------------------------- |
| Chunked Transfer Encoding | ⭐⭐⭐     | 1-2 weeks           | HIGH (common in HTTP/1.1)  |
| Keep-alive                | ⭐⭐⭐⭐   | 2-3 weeks           | HIGH (performance)         |
| Range Requests            | ⭐⭐⭐     | 1 week              | MEDIUM (useful for media)  |
| Compression               | ⭐⭐⭐     | 1 week              | MEDIUM (reduces bandwidth) |
| Multipart Parsing         | ⭐⭐⭐⭐   | 2 weeks             | MEDIUM (file uploads)      |
| SSL/TLS                   | ⭐⭐⭐⭐   | 3-4 weeks           | LOW (use reverse proxy)    |
| HTTP/2                    | ⭐⭐⭐⭐⭐ | 2-3 months          | LOW (complete rewrite)     |

**Recommended Implementation Order**:

1. **Chunked encoding** - Required for proper HTTP/1.1 compliance
2. **Keep-alive** - Major performance improvement
3. **Compression** - Easy win for bandwidth reduction
4. **Range requests** - If serving large files/media
5. **Multipart** - If building web forms with uploads
6. **SSL/TLS** - Only if can't use reverse proxy
7. **HTTP/2** - Only if specific use case requires it

Would you like me to implement any of these features? I'd recommend starting with **chunked encoding** as it's the most critical for HTTP/1.1 compliance.
