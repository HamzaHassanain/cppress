#include <gtest/gtest.h>

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "../includes.hpp"
#include "shared/includes/thread_pool.hpp"
#include "shared/includes/utils.hpp"
#include "sockets/includes.hpp"
using namespace cppress::sockets;
cppress::shared::thread_pool pool(std::thread::hardware_concurrency());

const std::vector<std::string> allowed_methods = {
    "GET",     "POST",     "PUT",   "DELETE", "HEAD", "OPTIONS", "PATCH", "TRACE",
    "CONNECT", "PROPFIND", "MKCOL", "COPY",   "MOVE", "LOCK",    "UNLOCK"};

class HttpServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        cppress::http::config::MAX_IDLE_TIME_SECONDS = std::chrono::seconds(5);
        cppress::http::config::MAX_HEADER_SIZE = 1024 * 32;
        cppress::http::config::MAX_BODY_SIZE = 1024 * 20;  // 20 KB

        if (!cppress::sockets::initialize_socket_library()) {
            throw std::runtime_error("Failed to initialize socket library.");
        }
    }

    void TearDown() override { cppress::sockets::cleanup_socket_library(); }
};

TEST(HttpServerTest, BasicRequestResponse) {
    cppress::http::http_server server(9986);
    bool started = false;
    bool stopped = false;

    server.set_listen_success_callback([&started]() { started = true; });
    server.set_server_stopped_callback([&stopped]() { stopped = true; });

    server.set_request_callback(
        [](cppress::http::http_request& req, cppress::http::http_response& res) {
            res.add_header("Content-Type", "text/plain");
            res.set_body("Hello, " + req.get_uri() + " " + req.get_method());
            res.set_status(200, "OK");
            res.send();
            res.end();
        });
    std::thread server_thread([&server]() { server.listen(); });

    std::thread stop_thread([&server]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        server.shutdown();
    });

    std::vector<std::thread> clients;
    for (int i = 0; i < 10; ++i) {
        clients.emplace_back([i]() {
            try {
                cppress::sockets::connection conn;
                conn.connect(cppress::sockets::socket_address(port(9986), ip_address("0.0.0.0")));
                conn.write(data_buffer("GET /test" + std::to_string(i) +
                                       " HTTP/1.1\r\nHost: localhost\r\n\r\n"));
                auto data = conn.read();
                auto response = std::string(data.to_string());

                EXPECT_NE(response.find("200 OK"), std::string::npos);
                EXPECT_NE(response.find("Hello, /test" + std::to_string(i) + " GET"),
                          std::string::npos);

            } catch (const std::exception& e) {
                FAIL() << "Client socket creation failed: " << e.what();
            }
        });
    }
    for (auto& client : clients) {
        client.join();
    }
    stop_thread.join();
    server_thread.join();
}

TEST(HttpServerTest, LargePayloadWithMultipleHeadersAndRouting) {
    cppress::http::http_server server(9985);

    std::atomic<int> post_count{0};
    std::atomic<int> get_count{0};
    std::atomic<int> put_count{0};
    std::atomic<int> delete_count{0};
    std::mutex data_mutex;
    std::map<std::string, std::string> server_storage;

    server.set_request_callback([&](cppress::http::http_request& req,
                                    cppress::http::http_response& res) {
        std::string method = req.get_method();
        std::string uri = req.get_uri();
        std::string body = req.get_body();

        if (method == "POST" && uri == "/api/data") {
            post_count++;
            std::lock_guard<std::mutex> lock(data_mutex);

            std::string key = "item_" + std::to_string(post_count.load());
            server_storage[key] = body;

            res.set_status(201, "Created");
            res.add_header("Content-Type", "application/json");
            res.add_header("Location", "/api/data/" + key);
            res.add_header("X-Item-Count", std::to_string(server_storage.size()));
            res.set_body("{\"status\":\"created\",\"key\":\"" + key +
                         "\",\"size\":" + std::to_string(body.length()) + "}");

        } else if (method == "GET" && uri.find("/api/data/") == 0) {
            get_count++;
            std::string key = uri.substr(10);
            std::lock_guard<std::mutex> lock(data_mutex);

            if (server_storage.find(key) != server_storage.end()) {
                res.set_status(200, "OK");
                res.add_header("Content-Type", "application/json");
                res.add_header("Cache-Control", "no-cache");
                res.add_header("X-Data-Size", std::to_string(server_storage[key].length()));
                res.set_body("{\"key\":\"" + key + "\",\"data\":\"" + server_storage[key] + "\"}");
            } else {
                res.set_status(404, "Not Found");
                res.add_header("Content-Type", "application/json");
                res.set_body("{\"error\":\"Item not found\"}");
            }

        } else if (method == "PUT" && uri.find("/api/data/") == 0) {
            put_count++;
            std::string key = uri.substr(10);
            std::lock_guard<std::mutex> lock(data_mutex);

            if (server_storage.find(key) != server_storage.end()) {
                server_storage[key] = body;
                res.set_status(200, "OK");
                res.add_header("Content-Type", "application/json");
                res.set_body("{\"status\":\"updated\",\"key\":\"" + key + "\"}");
            } else {
                res.set_status(404, "Not Found");
                res.add_header("Content-Type", "application/json");
                res.set_body("{\"error\":\"Item not found\"}");
            }

        } else if (method == "DELETE" && uri.find("/api/data/") == 0) {
            delete_count++;
            std::string key = uri.substr(10);
            std::lock_guard<std::mutex> lock(data_mutex);

            if (server_storage.find(key) != server_storage.end()) {
                server_storage.erase(key);
                res.set_status(204, "No Content");
                res.add_header("X-Items-Remaining", std::to_string(server_storage.size()));
            } else {
                res.set_status(404, "Not Found");
                res.add_header("Content-Type", "application/json");
                res.set_body("{\"error\":\"Item not found\"}");
            }

        } else if (method == "GET" && uri == "/api/stats") {
            res.set_status(200, "OK");
            res.add_header("Content-Type", "application/json");
            res.set_body("{\"posts\":" + std::to_string(post_count.load()) +
                         ",\"gets\":" + std::to_string(get_count.load()) +
                         ",\"puts\":" + std::to_string(put_count.load()) +
                         ",\"deletes\":" + std::to_string(delete_count.load()) +
                         ",\"stored_items\":" + std::to_string(server_storage.size()) + "}");
        } else {
            res.set_status(404, "Not Found");
            res.add_header("Content-Type", "text/plain");
            res.set_body("Route not found");
        }

        res.send();
        res.end();
    });

    std::thread server_thread([&server]() { server.listen(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::string large_payload_1(5000, 'A');
    std::string large_payload_2(8000, 'B');
    std::string large_payload_3(3000, 'C');

    std::vector<std::thread> client_threads;
    std::atomic<int> successful_operations{0};

    for (int i = 0; i < 15; ++i) {
        client_threads.emplace_back([&, i]() {
            try {
                std::string payload = (i % 3 == 0)   ? large_payload_1
                                      : (i % 3 == 1) ? large_payload_2
                                                     : large_payload_3;

                cppress::sockets::connection conn1;
                conn1.connect(
                    cppress::sockets::socket_address(port(9985), ip_address("127.0.0.1")));

                std::string post_request =
                    "POST /api/data HTTP/1.1\r\n"
                    "Host: localhost\r\n"
                    "Content-Type: application/octet-stream\r\n"
                    "Content-Length: " +
                    std::to_string(payload.length()) +
                    "\r\n"
                    "X-Client-ID: client-" +
                    std::to_string(i) +
                    "\r\n"
                    "X-Request-Type: create\r\n"
                    "\r\n" +
                    payload;

                conn1.write(data_buffer(post_request));
                auto post_response = std::string(conn1.read().to_string());

                if (post_response.find("201 Created") != std::string::npos) {
                    size_t key_pos = post_response.find("\"key\":\"");
                    if (key_pos != std::string::npos) {
                        size_t key_start = key_pos + 7;
                        size_t key_end = post_response.find("\"", key_start);
                        std::string key = post_response.substr(key_start, key_end - key_start);

                        std::this_thread::sleep_for(std::chrono::milliseconds(10));

                        cppress::sockets::connection conn2;
                        conn2.connect(
                            cppress::sockets::socket_address(port(9985), ip_address("127.0.0.1")));

                        std::string get_request = "GET /api/data/" + key +
                                                  " HTTP/1.1\r\n"
                                                  "Host: localhost\r\n"
                                                  "Accept: application/json\r\n"
                                                  "\r\n";

                        conn2.write(data_buffer(get_request));
                        auto get_response = std::string(conn2.read().to_string());

                        if (get_response.find("200 OK") != std::string::npos) {
                            successful_operations++;
                        }
                    }
                }

            } catch (const std::exception& e) {
                std::cerr << "Client " << i << " error: " << e.what() << std::endl;
            }
        });
    }

    for (auto& thread : client_threads) {
        thread.join();
    }

    try {
        cppress::sockets::connection stats_conn;
        stats_conn.connect(cppress::sockets::socket_address(port(9985), ip_address("127.0.0.1")));
        stats_conn.write(data_buffer("GET /api/stats HTTP/1.1\r\nHost: localhost\r\n\r\n"));
        auto stats_response = std::string(stats_conn.read().to_string());

        EXPECT_NE(stats_response.find("200 OK"), std::string::npos);
        EXPECT_NE(stats_response.find("\"posts\":"), std::string::npos);
        EXPECT_NE(stats_response.find("\"gets\":"), std::string::npos);
    } catch (const std::exception& e) {
        FAIL() << "Stats request failed: " << e.what();
    }

    EXPECT_GT(post_count.load(), 0);
    EXPECT_GT(get_count.load(), 0);
    EXPECT_GT(successful_operations.load(), 0);

    server.shutdown();
    server_thread.join();
}

TEST(HttpServerTest, ConcurrentConnectionsWithDifferentResponseTypesAndTimings) {
    cppress::http::http_server server(9984);

    std::atomic<int> fast_requests{0};
    std::atomic<int> slow_requests{0};
    std::atomic<int> streaming_requests{0};
    std::mutex timing_mutex;
    std::vector<std::chrono::milliseconds> response_times;

    server.set_request_callback([&](cppress::http::http_request& req,
                                    cppress::http::http_response& res) {
        std::string uri = req.get_uri();
        auto request_headers = req.get_headers();

        if (uri == "/fast") {
            fast_requests++;
            res.set_status(200, "OK");
            res.add_header("Content-Type", "text/plain");
            res.add_header("X-Response-Type", "fast");
            res.add_header("X-Processing-Time", "instant");
            res.set_body("Fast response");

        } else if (uri == "/slow") {
            slow_requests++;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            res.set_status(200, "OK");
            res.add_header("Content-Type", "application/json");
            res.add_header("X-Response-Type", "slow");
            res.add_header("X-Processing-Time", "100ms");
            res.set_body(
                "{\"message\":\"This response was deliberately delayed\",\"delay_ms\":100}");

        } else if (uri == "/large") {
            streaming_requests++;

            std::string large_body(15000, 'X');
            for (size_t i = 0; i < large_body.size(); i += 100) {
                large_body[i] = '\n';
            }

            res.set_status(200, "OK");
            res.add_header("Content-Type", "text/plain");
            res.add_header("X-Response-Type", "large");
            res.add_header("X-Body-Size", std::to_string(large_body.length()));
            res.set_body(large_body);

        } else if (uri == "/echo") {
            std::string body = req.get_body();
            auto content_type_headers = req.get_header("Content-Type");

            res.set_status(200, "OK");
            if (!content_type_headers.empty()) {
                res.add_header("Content-Type", content_type_headers[0]);
            }
            res.add_header("X-Response-Type", "echo");
            res.add_header("X-Echo-Size", std::to_string(body.length()));
            res.set_body(body);

        } else if (uri == "/headers") {
            std::string headers_info = "{\"headers\":[";
            bool first = true;
            for (const auto& header : request_headers) {
                if (!first)
                    headers_info += ",";
                headers_info +=
                    "{\"name\":\"" + header.first + "\",\"value\":\"" + header.second + "\"}";
                first = false;
            }
            headers_info += "]}";

            res.set_status(200, "OK");
            res.add_header("Content-Type", "application/json");
            res.add_header("X-Response-Type", "headers-analysis");
            res.add_header("X-Header-Count", std::to_string(request_headers.size()));
            res.set_body(headers_info);

        } else if (uri.find("/status/") == 0) {
            std::string status_code_str = uri.substr(8);
            int status_code = std::stoi(status_code_str);

            std::map<int, std::string> status_messages = {{200, "OK"},
                                                          {201, "Created"},
                                                          {204, "No Content"},
                                                          {400, "Bad Request"},
                                                          {401, "Unauthorized"},
                                                          {403, "Forbidden"},
                                                          {404, "Not Found"},
                                                          {500, "Internal Server Error"},
                                                          {503, "Service Unavailable"}};

            std::string message =
                status_messages.count(status_code) ? status_messages[status_code] : "Custom Status";

            res.set_status(status_code, message);
            res.add_header("Content-Type", "application/json");
            res.add_header("X-Custom-Status", "true");
            res.set_body("{\"status\":" + std::to_string(status_code) + ",\"message\":\"" +
                         message + "\"}");

        } else {
            res.set_status(404, "Not Found");
            res.add_header("Content-Type", "text/plain");
            res.set_body("Endpoint not found: " + uri);
        }

        res.send();
        res.end();
    });

    std::thread server_thread([&server]() { server.listen(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::vector<std::thread> test_threads;
    std::atomic<int> fast_success{0};
    std::atomic<int> slow_success{0};
    std::atomic<int> large_success{0};
    std::atomic<int> echo_success{0};
    std::atomic<int> headers_success{0};
    std::atomic<int> status_success{0};

    for (int i = 0; i < 5; ++i) {
        test_threads.emplace_back([&, i]() {
            try {
                auto start = std::chrono::steady_clock::now();

                cppress::sockets::connection conn;
                conn.connect(cppress::sockets::socket_address(port(9984), ip_address("127.0.0.1")));
                conn.write(data_buffer("GET /fast HTTP/1.1\r\nHost: localhost\r\n\r\n"));
                auto response = std::string(conn.read().to_string());

                auto end = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

                {
                    std::lock_guard<std::mutex> lock(timing_mutex);
                    response_times.push_back(duration);
                }

                if (response.find("200 OK") != std::string::npos &&
                    response.find("Fast response") != std::string::npos) {
                    fast_success++;
                }
            } catch (const std::exception& e) {
                std::cerr << "Fast request error: " << e.what() << std::endl;
            }
        });
    }

    for (int i = 0; i < 5; ++i) {
        test_threads.emplace_back([&, i]() {
            try {
                cppress::sockets::connection conn;
                conn.connect(cppress::sockets::socket_address(port(9984), ip_address("127.0.0.1")));
                conn.write(data_buffer("GET /slow HTTP/1.1\r\nHost: localhost\r\n\r\n"));
                auto response = std::string(conn.read().to_string());

                if (response.find("200 OK") != std::string::npos &&
                    response.find("deliberately delayed") != std::string::npos) {
                    slow_success++;
                }
            } catch (const std::exception& e) {
                std::cerr << "Slow request error: " << e.what() << std::endl;
            }
        });
    }

    for (int i = 0; i < 3; ++i) {
        test_threads.emplace_back([&, i]() {
            try {
                cppress::sockets::connection conn;
                conn.connect(cppress::sockets::socket_address(port(9984), ip_address("127.0.0.1")));
                conn.write(data_buffer("GET /large HTTP/1.1\r\nHost: localhost\r\n\r\n"));
                auto response = std::string(conn.read().to_string());

                if (response.find("200 OK") != std::string::npos &&
                    response.find(cppress::shared::to_uppercase("X-Body-Size")) !=
                        std::string::npos) {
                    // if (response.size() > 15000) {
                    large_success++;
                }
            } catch (const std::exception& e) {
                std::cerr << "Large request error: " << e.what() << std::endl;
            }
        });
    }

    for (int i = 0; i < 4; ++i) {
        test_threads.emplace_back([&, i]() {
            try {
                std::string echo_data =
                    "Echo test data " + std::to_string(i) + " with some content";
                std::string request =
                    "POST /echo HTTP/1.1\r\n"
                    "Host: localhost\r\n"
                    "Content-Type: text/plain\r\n"
                    "Content-Length: " +
                    std::to_string(echo_data.length()) +
                    "\r\n"
                    "\r\n" +
                    echo_data;

                cppress::sockets::connection conn;
                conn.connect(cppress::sockets::socket_address(port(9984), ip_address("127.0.0.1")));
                conn.write(data_buffer(request));
                auto response = std::string(conn.read().to_string());

                if (response.find("200 OK") != std::string::npos &&
                    response.find(echo_data) != std::string::npos) {
                    echo_success++;
                }
            } catch (const std::exception& e) {
                std::cerr << "Echo request error: " << e.what() << std::endl;
            }
        });
    }

    test_threads.emplace_back([&]() {
        try {
            std::string request =
                "GET /headers HTTP/1.1\r\n"
                "Host: localhost\r\n"
                "User-Agent: TestClient/1.0\r\n"
                "Accept: application/json\r\n"
                "X-Custom-Header-1: value1\r\n"
                "X-Custom-Header-2: value2\r\n"
                "\r\n";

            cppress::sockets::connection conn;
            conn.connect(cppress::sockets::socket_address(port(9984), ip_address("127.0.0.1")));
            conn.write(data_buffer(request));
            auto response = std::string(conn.read().to_string());

            if (response.find("200 OK") != std::string::npos &&
                response.find("headers-analysis") != std::string::npos) {
                headers_success++;
            }
        } catch (const std::exception& e) {
            std::cerr << "Headers request error: " << e.what() << std::endl;
        }
    });

    std::vector<int> status_codes = {200, 201, 204, 400, 404, 500};
    for (int code : status_codes) {
        test_threads.emplace_back([&, code]() {
            try {
                cppress::sockets::connection conn;
                conn.connect(cppress::sockets::socket_address(port(9984), ip_address("127.0.0.1")));
                std::string request =
                    "GET /status/" + std::to_string(code) + " HTTP/1.1\r\nHost: localhost\r\n\r\n";
                conn.write(data_buffer(request));
                auto response = std::string(conn.read().to_string());

                if (response.find(std::to_string(code)) != std::string::npos) {
                    status_success++;
                }
            } catch (const std::exception& e) {
                std::cerr << "Status " << code << " request error: " << e.what() << std::endl;
            }
        });
    }

    for (auto& thread : test_threads) {
        thread.join();
    }

    EXPECT_GT(fast_requests.load(), 0);
    EXPECT_GT(slow_requests.load(), 0);
    EXPECT_GT(streaming_requests.load(), 0);
    EXPECT_GT(fast_success.load(), 0);
    EXPECT_GT(slow_success.load(), 0);
    EXPECT_GT(large_success.load(), 0);
    EXPECT_GT(echo_success.load(), 0);
    EXPECT_EQ(headers_success.load(), 1);
    EXPECT_EQ(status_success.load(), status_codes.size());

    EXPECT_GE(response_times.size(), 0);

    server.shutdown();
    server_thread.join();
}

TEST(HttpServerTest, ConnectionReuseForSameRequest) {
    cppress::http::http_server server(9983);

    std::atomic<int> request_count{0};

    server.set_request_callback(
        [&](cppress::http::http_request& req, cppress::http::http_response& res) {
            request_count++;
            res.set_status(200, "OK");
            res.add_header("Content-Type", "text/plain");
            res.add_header("X-Request-Count", std::to_string(request_count.load()));
            res.set_body("Request number: " + std::to_string(request_count.load()));
            res.send();
        });

    std::thread server_thread([&server]() { server.listen(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    try {
        cppress::sockets::connection conn;
        conn.connect(cppress::sockets::socket_address(port(9983), ip_address("127.0.0.1")));
        conn.write(data_buffer("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"));
        const std::string response1 = std::string(conn.read().to_string());
        conn.write(data_buffer("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"));
        const std::string response2 = std::string(conn.read().to_string());
        conn.write(data_buffer("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"));
        const std::string response3 = std::string(conn.read().to_string());
        conn.write(data_buffer("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"));
        const std::string response4 = std::string(conn.read().to_string());
        conn.write(data_buffer("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"));
        const std::string response5 = std::string(conn.read().to_string());

        EXPECT_NE(response5.find("200 OK"), std::string::npos);
        // EXPECT_EQ(response.find("X-REQUEST-COUNT"), 0);
    } catch (const std::exception& e) {
        std::cerr << "Request error: " << e.what() << std::endl;
    }

    server.shutdown();
    server_thread.join();
}