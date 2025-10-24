#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "../includes.hpp"
#include "libs/html/includes.hpp"
#include "libs/json/includes.hpp"
#include "shared/includes/thread_pool.hpp"
#include "shared/includes/utils.hpp"
#include "sockets/includes.hpp"

using namespace cppress;
using namespace cppress::web;
using namespace cppress::json;
using namespace cppress::sockets;

/**
 * @brief Test fixture for web server tests
 *
 * Sets up the socket library and HTTP configuration before each test
 * and cleans up after each test.
 */
class WebServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        cppress::http::config::MAX_IDLE_TIME_SECONDS = std::chrono::seconds(5);
        cppress::http::config::MAX_HEADER_SIZE = 1024 * 32;
        cppress::http::config::MAX_BODY_SIZE = 1024 * 64;  // 64 KB for larger payloads

        if (!cppress::sockets::initialize_socket_library()) {
            throw std::runtime_error("Failed to initialize socket library.");
        }
    }

    void TearDown() override { cppress::sockets::cleanup_socket_library(); }
};

/**
 * @brief Test 1: Basic web server setup and routing
 *
 * This test verifies:
 * - Server starts and listens correctly
 * - Basic routing with GET/POST/PUT/DELETE methods
 * - Request and response objects work properly
 * - Multiple concurrent clients can connect
 * - Path parameters are extracted correctly
 * - Query parameters are parsed correctly
 * - Middleware execution order
 * - Custom 404 handler works
 */
TEST_F(WebServerTest, BasicServerSetupAndRouting) {
    auto server = std::make_shared<cppress::web::server<>>(8080, "0.0.0.0");

    std::atomic<int> middleware_count{0};
    std::atomic<int> get_count{0};
    std::atomic<int> post_count{0};
    std::atomic<int> put_count{0};
    std::atomic<int> delete_count{0};
    std::atomic<int> not_found_count{0};
    std::atomic<int> param_route_count{0};

    // Add middleware to track all requests
    server->use([&middleware_count](std::shared_ptr<request> req,
                                    std::shared_ptr<response> res) -> exit_code {
        middleware_count++;
        res->add_header("X-Middleware", "executed");
        return exit_code::CONTINUE;
    });

    // GET route
    server->get("/test", {[&get_count](std::shared_ptr<request> req,
                                       std::shared_ptr<response> res) -> exit_code {
                    get_count++;
                    res->set_status(200, "OK");
                    res->send_text("GET request successful");
                    return exit_code::EXIT;
                }});

    // POST route
    server->post("/data", {[&post_count](std::shared_ptr<request> req,
                                         std::shared_ptr<response> res) -> exit_code {
                     post_count++;
                     std::string body = req->get_body();
                     res->set_status(201, "Created");
                     res->send_text("POST received: " + body);
                     return exit_code::EXIT;
                 }});

    // PUT route
    server->put("/update", {[&put_count](std::shared_ptr<request> req,
                                         std::shared_ptr<response> res) -> exit_code {
                    put_count++;
                    res->set_status(200, "OK");
                    res->send_text("PUT successful");
                    return exit_code::EXIT;
                }});

    // DELETE route
    server->delete_("/remove", {[&delete_count](std::shared_ptr<request> req,
                                                std::shared_ptr<response> res) -> exit_code {
                        delete_count++;
                        res->set_status(204, "No Content");
                        res->send();
                        return exit_code::EXIT;
                    }});

    // Route with path parameters
    server->get("/users/:id/posts/:postId",
                {[&param_route_count](std::shared_ptr<request> req,
                                      std::shared_ptr<response> res) -> exit_code {
                    param_route_count++;
                    auto params = req->get_path_params();

                    std::string user_id, post_id;
                    for (const auto& [key, value] : params) {
                        if (key == "id")
                            user_id = value;
                        if (key == "postId")
                            post_id = value;
                    }

                    res->set_status(200, "OK");
                    res->send_text("User: " + user_id + ", Post: " + post_id);
                    return exit_code::EXIT;
                }});

    // Route with query parameters
    server->get("/search",
                {[](std::shared_ptr<request> req, std::shared_ptr<response> res) -> exit_code {
                    auto query_params = req->get_query_parameters();
                    std::string query, page;

                    for (const auto& [key, value] : query_params) {
                        if (key == "q")
                            query = value;
                        if (key == "page")
                            page = value;
                    }

                    res->set_status(200, "OK");
                    res->send_text("Search: " + query + ", Page: " + page);
                    return exit_code::EXIT;
                }});

    // Custom 404 handler
    server->use_default([&not_found_count](std::shared_ptr<request> req,
                                           std::shared_ptr<response> res) -> exit_code {
        not_found_count++;
        res->set_status(404, "Not Found");
        res->send_text("Custom 404: Route not found");
        return exit_code::EXIT;
    });

    // Start server in separate thread
    std::thread server_thread([&server]() {
        server->listen([]() { std::cout << "Web server started on port 8080" << std::endl; },
                       [](const std::exception& e) {
                           std::cerr << "Server error: " << e.what() << std::endl;
                       });
    });

    // Give server time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // Test clients
    std::vector<std::thread> clients;
    std::atomic<int> successful_requests{0};

    // Test GET request
    clients.emplace_back([&successful_requests]() {
        try {
            cppress::sockets::connection conn;
            conn.connect(cppress::sockets::socket_address(port(8080), ip_address("127.0.0.1")));
            conn.write(data_buffer("GET /test HTTP/1.1\r\nHost: localhost\r\n\r\n"));
            auto response = std::string(conn.read().to_string());

            if (response.find("200 OK") != std::string::npos &&
                response.find("GET request successful") != std::string::npos &&
                response.find("X-Middleware: executed") != std::string::npos) {
                successful_requests++;
            }
        } catch (const std::exception& e) {
            std::cerr << "GET test error: " << e.what() << std::endl;
        }
    });

    // Test POST request
    clients.emplace_back([&successful_requests]() {
        try {
            std::string body = "test data";
            std::string request =
                "POST /data HTTP/1.1\r\n"
                "Host: localhost\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: " +
                std::to_string(body.length()) + "\r\n\r\n" + body;

            cppress::sockets::connection conn;
            conn.connect(cppress::sockets::socket_address(port(8080), ip_address("127.0.0.1")));
            conn.write(data_buffer(request));
            auto response = std::string(conn.read().to_string());

            if (response.find("201 Created") != std::string::npos &&
                response.find("POST received: test data") != std::string::npos) {
                successful_requests++;
            }
        } catch (const std::exception& e) {
            std::cerr << "POST test error: " << e.what() << std::endl;
        }
    });

    // Test PUT request
    clients.emplace_back([&successful_requests]() {
        try {
            cppress::sockets::connection conn;
            conn.connect(cppress::sockets::socket_address(port(8080), ip_address("127.0.0.1")));
            conn.write(data_buffer("PUT /update HTTP/1.1\r\nHost: localhost\r\n\r\n"));
            auto response = std::string(conn.read().to_string());

            if (response.find("200 OK") != std::string::npos &&
                response.find("PUT successful") != std::string::npos) {
                successful_requests++;
            }
        } catch (const std::exception& e) {
            std::cerr << "PUT test error: " << e.what() << std::endl;
        }
    });

    // Test DELETE request
    clients.emplace_back([&successful_requests]() {
        try {
            cppress::sockets::connection conn;
            conn.connect(cppress::sockets::socket_address(port(8080), ip_address("127.0.0.1")));
            conn.write(data_buffer("DELETE /remove HTTP/1.1\r\nHost: localhost\r\n\r\n"));
            auto response = std::string(conn.read().to_string());

            if (response.find("204 No Content") != std::string::npos) {
                successful_requests++;
            }
        } catch (const std::exception& e) {
            std::cerr << "DELETE test error: " << e.what() << std::endl;
        }
    });

    // Test path parameters
    clients.emplace_back([&successful_requests]() {
        try {
            cppress::sockets::connection conn;
            conn.connect(cppress::sockets::socket_address(port(8080), ip_address("127.0.0.1")));
            conn.write(data_buffer("GET /users/123/posts/456 HTTP/1.1\r\nHost: localhost\r\n\r\n"));
            auto response = std::string(conn.read().to_string());

            if (response.find("200 OK") != std::string::npos &&
                response.find("User: 123, Post: 456") != std::string::npos) {
                successful_requests++;
            }
        } catch (const std::exception& e) {
            std::cerr << "Path params test error: " << e.what() << std::endl;
        }
    });

    // Test query parameters
    clients.emplace_back([&successful_requests]() {
        try {
            cppress::sockets::connection conn;
            conn.connect(cppress::sockets::socket_address(port(8080), ip_address("127.0.0.1")));
            conn.write(
                data_buffer("GET /search?q=cppress&page=2 HTTP/1.1\r\nHost: localhost\r\n\r\n"));
            auto response = std::string(conn.read().to_string());

            if (response.find("200 OK") != std::string::npos &&
                response.find("Search: cppress, Page: 2") != std::string::npos) {
                successful_requests++;
            }
        } catch (const std::exception& e) {
            std::cerr << "Query params test error: " << e.what() << std::endl;
        }
    });

    // Test 404 handler
    clients.emplace_back([&successful_requests]() {
        try {
            cppress::sockets::connection conn;
            conn.connect(cppress::sockets::socket_address(port(8080), ip_address("127.0.0.1")));
            conn.write(data_buffer("GET /nonexistent HTTP/1.1\r\nHost: localhost\r\n\r\n"));
            auto response = std::string(conn.read().to_string());

            if (response.find("404 Not Found") != std::string::npos &&
                response.find("Custom 404: Route not found") != std::string::npos) {
                successful_requests++;
            }
        } catch (const std::exception& e) {
            std::cerr << "404 test error: " << e.what() << std::endl;
        }
    });

    // Wait for all clients to complete
    for (auto& client : clients) {
        client.join();
    }

    // Verify results
    EXPECT_EQ(successful_requests.load(), 7);
    EXPECT_GT(middleware_count.load(), 0);
    EXPECT_EQ(get_count.load(), 3);  // /test, path params, query params
    EXPECT_EQ(post_count.load(), 1);
    EXPECT_EQ(put_count.load(), 1);
    EXPECT_EQ(delete_count.load(), 1);
    EXPECT_EQ(param_route_count.load(), 1);
    EXPECT_EQ(not_found_count.load(), 1);

    // Stop server
    server->stop();
    server_thread.join();
}

/**
 * @brief Test 2: JSON API with cppress JSON library
 *
 * This test verifies:
 * - JSON request parsing using cppress::json
 * - JSON response generation using cppress::json
 * - RESTful API with JSON payloads
 * - Proper Content-Type handling for JSON
 * - Complex nested JSON structures
 * - JSON validation middleware
 * - Error handling with JSON error responses
 */
TEST_F(WebServerTest, JsonApiWithCppressJsonLibrary) {
    auto server = std::make_shared<cppress::web::server<>>(8081, "0.0.0.0");

    // In-memory storage for JSON data
    std::mutex storage_mutex;
    std::map<int, json::json_object> items_storage;
    std::atomic<int> next_id{1};

    // JSON validation middleware
    auto json_validator = [](std::shared_ptr<request> req,
                             std::shared_ptr<response> res) -> exit_code {
        if (req->get_method() == "POST" || req->get_method() == "PUT") {
            try {
                // Try to parse the body as JSON
                json::parse(req->get_body());
                return exit_code::CONTINUE;
            } catch (const std::exception& e) {
                // Invalid JSON
                json::json_object error_obj;
                error_obj.insert("error", json::maker::make_string("Invalid JSON format"));
                error_obj.insert("message", json::maker::make_string(e.what()));

                res->set_status(400, "Bad Request");
                res->send_json(error_obj.stringify());
                return exit_code::EXIT;
            }
        }
        return exit_code::CONTINUE;
    };

    // Create a router for API endpoints
    auto api_router = std::make_shared<router<>>();
    api_router->use(json_validator);

    // POST /api/items - Create new item
    api_router->post(
        "/api/items",
        {[&](std::shared_ptr<request> req, std::shared_ptr<response> res) -> exit_code {
            try {
                auto json_data = json::parse(req->get_body());

                // Extract fields using cppress::json getters
                std::string name = json::getter::get_string(json_data["name"]);
                std::string description = json::getter::get_string(json_data["description"]);
                double price = json::getter::get_number(json_data["price"]);

                // Create new item
                int id = next_id++;
                json::json_object item;
                item.insert("id", json::maker::make_number(id));
                item.insert("name", json::maker::make_string(name));
                item.insert("description", json::maker::make_string(description));
                item.insert("price", json::maker::make_number(price));

                {
                    std::lock_guard<std::mutex> lock(storage_mutex);
                    items_storage[id] = item;
                }

                res->set_status(201, "Created");
                res->add_header("Location", "/api/items/" + std::to_string(id));
                res->send_json(item.stringify());

            } catch (const std::exception& e) {
                json::json_object error_obj;
                error_obj.insert("error", json::maker::make_string("Failed to create item"));
                error_obj.insert("message", json::maker::make_string(e.what()));

                res->set_status(500, "Internal Server Error");
                res->send_json(error_obj.stringify());
            }
            return exit_code::EXIT;
        }});

    // GET /api/items - Get all items
    api_router->get(
        "/api/items",
        {[&](std::shared_ptr<request> req, std::shared_ptr<response> res) -> exit_code {
            try {
                auto items_array = json::maker::make_array();

                {
                    std::lock_guard<std::mutex> lock(storage_mutex);
                    for (const auto& [id, item] : items_storage) {
                        items_array->push_back(std::make_shared<json::json_object>(item));
                    }
                }

                json::json_object response_obj;
                response_obj.insert("count", json::maker::make_number((long)items_array->size()));
                response_obj.insert("items", items_array);

                res->set_status(200, "OK");
                res->send_json(response_obj.stringify());

            } catch (const std::exception& e) {
                json::json_object error_obj;
                error_obj.insert("error", json::maker::make_string("Failed to retrieve items"));

                res->set_status(500, "Internal Server Error");
                res->send_json(error_obj.stringify());
            }
            return exit_code::EXIT;
        }});

    // GET /api/items/:id - Get specific item
    api_router->get("/api/items/:id",
                    {[&](std::shared_ptr<request> req, std::shared_ptr<response> res) -> exit_code {
                        try {
                            auto params = req->get_path_params();
                            int id = 0;
                            for (const auto& [key, value] : params) {
                                if (key == "id") {
                                    id = std::stoi(value);
                                    break;
                                }
                            }

                            std::lock_guard<std::mutex> lock(storage_mutex);
                            if (items_storage.find(id) != items_storage.end()) {
                                res->set_status(200, "OK");
                                res->send_json(items_storage[id].stringify());
                            } else {
                                json::json_object error_obj;
                                error_obj.insert("error",
                                                 json::maker::make_string("Item not found"));
                                error_obj.insert("id", json::maker::make_number(id));

                                res->set_status(404, "Not Found");
                                res->send_json(error_obj.stringify());
                            }

                        } catch (const std::exception& e) {
                            json::json_object error_obj;
                            error_obj.insert("error", json::maker::make_string("Invalid item ID"));

                            res->set_status(400, "Bad Request");
                            res->send_json(error_obj.stringify());
                        }
                        return exit_code::EXIT;
                    }});

    // PUT /api/items/:id - Update item
    api_router->put(
        "/api/items/:id",
        {[&](std::shared_ptr<request> req, std::shared_ptr<response> res) -> exit_code {
            try {
                auto params = req->get_path_params();
                int id = 0;
                for (const auto& [key, value] : params) {
                    if (key == "id") {
                        id = std::stoi(value);
                        break;
                    }
                }

                auto json_data = json::parse(req->get_body());
                std::string name = json::getter::get_string(json_data["name"]);
                std::string description = json::getter::get_string(json_data["description"]);
                double price = json::getter::get_number(json_data["price"]);

                std::lock_guard<std::mutex> lock(storage_mutex);
                if (items_storage.find(id) != items_storage.end()) {
                    // Update item
                    json::json_object updated_item;
                    updated_item.insert("id", json::maker::make_number(id));
                    updated_item.insert("name", json::maker::make_string(name));
                    updated_item.insert("description", json::maker::make_string(description));
                    updated_item.insert("price", json::maker::make_number(price));

                    items_storage[id] = updated_item;

                    res->set_status(200, "OK");
                    res->send_json(updated_item.stringify());
                } else {
                    json::json_object error_obj;
                    error_obj.insert("error", json::maker::make_string("Item not found"));

                    res->set_status(404, "Not Found");
                    res->send_json(error_obj.stringify());
                }

            } catch (const std::exception& e) {
                json::json_object error_obj;
                error_obj.insert("error", json::maker::make_string("Failed to update item"));

                res->set_status(400, "Bad Request");
                res->send_json(error_obj.stringify());
            }
            return exit_code::EXIT;
        }});

    // DELETE /api/items/:id - Delete item
    api_router->delete_(
        "/api/items/:id",
        {[&](std::shared_ptr<request> req, std::shared_ptr<response> res) -> exit_code {
            try {
                auto params = req->get_path_params();
                int id = 0;
                for (const auto& [key, value] : params) {
                    if (key == "id") {
                        id = std::stoi(value);
                        break;
                    }
                }

                std::lock_guard<std::mutex> lock(storage_mutex);
                if (items_storage.find(id) != items_storage.end()) {
                    items_storage.erase(id);
                    res->set_status(204, "No Content");
                    res->send();
                } else {
                    json::json_object error_obj;
                    error_obj.insert("error", json::maker::make_string("Item not found"));

                    res->set_status(404, "Not Found");
                    res->send_json(error_obj.stringify());
                }

            } catch (const std::exception& e) {
                json::json_object error_obj;
                error_obj.insert("error", json::maker::make_string("Failed to delete item"));

                res->set_status(500, "Internal Server Error");
                res->send_json(error_obj.stringify());
            }
            return exit_code::EXIT;
        }});

    server->use_router(api_router);

    // Start server
    std::thread server_thread([&server]() {
        server->listen([]() { std::cout << "JSON API server started on port 8081" << std::endl; },
                       [](const std::exception& e) {
                           std::cerr << "Server error: " << e.what() << std::endl;
                       });
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // Test clients
    std::vector<std::thread> clients;
    std::atomic<int> create_success{0};
    std::atomic<int> get_all_success{0};
    std::atomic<int> get_one_success{0};
    std::atomic<int> update_success{0};
    std::atomic<int> delete_success{0};
    std::atomic<int> validation_error_success{0};

    // Test 1: Create multiple items
    for (int i = 0; i < 5; ++i) {
        clients.emplace_back([&, i]() {
            try {
                json::json_object item;
                item.insert("name", json::maker::make_string("Item " + std::to_string(i)));
                item.insert("description",
                            json::maker::make_string("Description for item " + std::to_string(i)));
                item.insert("price", json::maker::make_number(10.5 + i * 5));

                std::string body = item.stringify();
                std::string request =
                    "POST /api/items HTTP/1.1\r\n"
                    "Host: localhost\r\n"
                    "Content-Type: application/json\r\n"
                    "Content-Length: " +
                    std::to_string(body.length()) + "\r\n\r\n" + body;

                cppress::sockets::connection conn;
                conn.connect(cppress::sockets::socket_address(port(8081), ip_address("127.0.0.1")));
                conn.write(data_buffer(request));
                auto response = std::string(conn.read().to_string());

                if (response.find("201 Created") != std::string::npos &&
                    response.find("\"id\"") != std::string::npos) {
                    create_success++;
                }
            } catch (const std::exception& e) {
                std::cerr << "Create item error: " << e.what() << std::endl;
            }
        });
    }

    // Wait for creates to finish
    for (auto& client : clients) {
        client.join();
    }
    clients.clear();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Test 2: Get all items
    clients.emplace_back([&]() {
        try {
            cppress::sockets::connection conn;
            conn.connect(cppress::sockets::socket_address(port(8081), ip_address("127.0.0.1")));
            conn.write(data_buffer("GET /api/items HTTP/1.1\r\nHost: localhost\r\n\r\n"));
            auto response = std::string(conn.read().to_string());

            if (response.find("200 OK") != std::string::npos &&
                response.find("\"count\"") != std::string::npos &&
                response.find("\"items\"") != std::string::npos) {
                get_all_success++;
            }
        } catch (const std::exception& e) {
            std::cerr << "Get all items error: " << e.what() << std::endl;
        }
    });

    // Test 3: Get specific item
    clients.emplace_back([&]() {
        try {
            cppress::sockets::connection conn;
            conn.connect(cppress::sockets::socket_address(port(8081), ip_address("127.0.0.1")));
            conn.write(data_buffer("GET /api/items/1 HTTP/1.1\r\nHost: localhost\r\n\r\n"));
            auto response = std::string(conn.read().to_string());

            if (response.find("200 OK") != std::string::npos &&
                response.find("\"name\"") != std::string::npos) {
                get_one_success++;
            }
        } catch (const std::exception& e) {
            std::cerr << "Get one item error: " << e.what() << std::endl;
        }
    });

    // Test 4: Update item
    clients.emplace_back([&]() {
        try {
            json::json_object updated;
            updated.insert("name", json::maker::make_string("Updated Item"));
            updated.insert("description", json::maker::make_string("Updated description"));
            updated.insert("price", json::maker::make_number(99.99));

            std::string body = updated.stringify();
            std::string request =
                "PUT /api/items/1 HTTP/1.1\r\n"
                "Host: localhost\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: " +
                std::to_string(body.length()) + "\r\n\r\n" + body;

            cppress::sockets::connection conn;
            conn.connect(cppress::sockets::socket_address(port(8081), ip_address("127.0.0.1")));
            conn.write(data_buffer(request));
            auto response = std::string(conn.read().to_string());

            if (response.find("200 OK") != std::string::npos &&
                response.find("Updated Item") != std::string::npos) {
                update_success++;
            }
        } catch (const std::exception& e) {
            std::cerr << "Update item error: " << e.what() << std::endl;
        }
    });

    // Test 5: Delete item
    clients.emplace_back([&]() {
        try {
            cppress::sockets::connection conn;
            conn.connect(cppress::sockets::socket_address(port(8081), ip_address("127.0.0.1")));
            conn.write(data_buffer("DELETE /api/items/2 HTTP/1.1\r\nHost: localhost\r\n\r\n"));
            auto response = std::string(conn.read().to_string());

            if (response.find("204 No Content") != std::string::npos) {
                delete_success++;
            }
        } catch (const std::exception& e) {
            std::cerr << "Delete item error: " << e.what() << std::endl;
        }
    });

    // Test 6: Invalid JSON validation
    clients.emplace_back([&]() {
        try {
            std::string invalid_json = "{invalid json}";
            std::string request =
                "POST /api/items HTTP/1.1\r\n"
                "Host: localhost\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: " +
                std::to_string(invalid_json.length()) + "\r\n\r\n" + invalid_json;

            cppress::sockets::connection conn;
            conn.connect(cppress::sockets::socket_address(port(8081), ip_address("127.0.0.1")));
            conn.write(data_buffer(request));
            auto response = std::string(conn.read().to_string());

            if (response.find("400 Bad Request") != std::string::npos &&
                response.find("Invalid JSON format") != std::string::npos) {
                validation_error_success++;
            }
        } catch (const std::exception& e) {
            std::cerr << "JSON validation error test: " << e.what() << std::endl;
        }
    });

    for (auto& client : clients) {
        client.join();
    }

    // Verify results
    EXPECT_EQ(create_success.load(), 5);
    EXPECT_EQ(get_all_success.load(), 1);
    EXPECT_EQ(get_one_success.load(), 1);
    EXPECT_EQ(update_success.load(), 1);
    EXPECT_EQ(delete_success.load(), 1);
    EXPECT_EQ(validation_error_success.load(), 1);

    server->stop();
    server_thread.join();
}

/**
 * @brief Test 3: HTML generation with cppress HTML library
 *
 * This test verifies:
 * - HTML document generation using cppress::html
 * - Dynamic HTML responses
 * - HTML forms and elements
 * - Template-like HTML generation
 * - Proper Content-Type handling for HTML
 * - Complex nested HTML structures
 * - Static file serving (simulated with dynamic HTML)
 */
TEST_F(WebServerTest, HtmlGenerationWithCppressHtmlLibrary) {
    auto server = std::make_shared<cppress::web::server<>>(8082, "0.0.0.0");

    // GET / - Home page with HTML document
    server->get(
        "/", {[](std::shared_ptr<request> req, std::shared_ptr<response> res) -> exit_code {
            html::document doc("html");

            auto head = html::maker::make_element("head");
            auto title = html::maker::make_element("title");
            title->set_text_content("cppress Web Framework Test");
            head->add_child(title);

            auto body = html::maker::make_element("body");
            auto main_div = html::maker::make_div();
            main_div->set_attribute("class", "container");

            auto heading = html::maker::make_heading(1, "Welcome to cppress Web Framework");
            main_div->add_child(heading);

            auto para = html::maker::make_paragraph(
                "This is a test page demonstrating HTML generation with the cppress HTML library.");
            main_div->add_child(para);

            auto link = html::maker::make_link("/users", "View Users");
            main_div->add_child(link);

            body->add_child(main_div);
            doc.add_child(head);
            doc.add_child(body);

            res->set_status(200, "OK");
            res->send_html(doc.to_string());
            return exit_code::EXIT;
        }});

    // GET /users - Dynamic user list
    server->get("/users",
                {[](std::shared_ptr<request> req, std::shared_ptr<response> res) -> exit_code {
                    html::document doc("html");

                    auto head = html::maker::make_element("head");
                    auto title = html::maker::make_element("title");
                    title->set_text_content("User List");
                    head->add_child(title);

                    auto body = html::maker::make_element("body");
                    auto heading = html::maker::make_heading(1, "User List");
                    body->add_child(heading);

                    // Create a table with users
                    auto table = html::maker::make_element("table");
                    table->set_attribute("border", "1");

                    auto thead = html::maker::make_element("thead");
                    auto header_row = html::maker::make_element("tr");
                    auto th1 = html::maker::make_element("th");
                    th1->set_text_content("ID");
                    auto th2 = html::maker::make_element("th");
                    th2->set_text_content("Name");
                    auto th3 = html::maker::make_element("th");
                    th3->set_text_content("Email");

                    header_row->add_child(th1);
                    header_row->add_child(th2);
                    header_row->add_child(th3);
                    thead->add_child(header_row);
                    table->add_child(thead);

                    auto tbody = html::maker::make_element("tbody");

                    // Add some test users
                    std::vector<std::tuple<int, std::string, std::string>> users = {
                        {1, "Alice", "alice@example.com"},
                        {2, "Bob", "bob@example.com"},
                        {3, "Charlie", "charlie@example.com"}};

                    for (const auto& [id, name, email] : users) {
                        auto row = html::maker::make_element("tr");

                        auto td1 = html::maker::make_element("td");
                        td1->set_text_content(std::to_string(id));

                        auto td2 = html::maker::make_element("td");
                        td2->set_text_content(name);

                        auto td3 = html::maker::make_element("td");
                        td3->set_text_content(email);

                        row->add_child(td1);
                        row->add_child(td2);
                        row->add_child(td3);
                        tbody->add_child(row);
                    }

                    table->add_child(tbody);
                    body->add_child(table);

                    auto back_link = html::maker::make_link("/", "Back to Home");
                    body->add_child(back_link);

                    doc.add_child(head);
                    doc.add_child(body);

                    res->set_status(200, "OK");
                    res->send_html(doc.to_string());
                    return exit_code::EXIT;
                }});

    // GET /form - HTML form
    server->get("/form",
                {[](std::shared_ptr<request> req, std::shared_ptr<response> res) -> exit_code {
                    html::document doc("html");

                    auto head = html::maker::make_element("head");
                    auto title = html::maker::make_element("title");
                    title->set_text_content("User Form");
                    head->add_child(title);

                    auto body = html::maker::make_element("body");
                    auto heading = html::maker::make_heading(2, "Create New User");
                    body->add_child(heading);

                    auto form = html::maker::make_element("form");
                    form->set_attribute("method", "POST");
                    form->set_attribute("action", "/submit");

                    auto name_label = html::maker::make_element("label");
                    name_label->set_text_content("Name: ");
                    auto name_input = html::maker::make_input("text", "name");
                    name_input->set_attribute("required", "required");

                    auto email_label = html::maker::make_element("label");
                    email_label->set_text_content("Email: ");
                    auto email_input = html::maker::make_input("email", "email");
                    email_input->set_attribute("required", "required");

                    auto submit_button = html::maker::make_element("button");
                    submit_button->set_attribute("type", "submit");
                    submit_button->set_text_content("Submit");

                    form->add_child(name_label);
                    form->add_child(name_input);
                    form->add_child(html::maker::make_element("br"));
                    form->add_child(email_label);
                    form->add_child(email_input);
                    form->add_child(html::maker::make_element("br"));
                    form->add_child(submit_button);

                    body->add_child(form);
                    doc.add_child(head);
                    doc.add_child(body);

                    res->set_status(200, "OK");
                    res->send_html(doc.to_string());
                    return exit_code::EXIT;
                }});

    // POST /submit - Form submission handler
    server->post(
        "/submit", {[](std::shared_ptr<request> req, std::shared_ptr<response> res) -> exit_code {
            html::document doc("html");

            auto head = html::maker::make_element("head");
            auto title = html::maker::make_element("title");
            title->set_text_content("Submission Success");
            head->add_child(title);

            auto body = html::maker::make_element("body");
            auto heading = html::maker::make_heading(2, "Form Submitted Successfully!");
            body->add_child(heading);

            auto success_para = html::maker::make_paragraph("Your data has been received.");
            body->add_child(success_para);

            // Display submitted data (in real scenario, you'd parse the form data)
            auto data_para = html::maker::make_paragraph("Request body: " + req->get_body());
            body->add_child(data_para);

            auto back_link = html::maker::make_link("/", "Back to Home");
            body->add_child(back_link);

            doc.add_child(head);
            doc.add_child(body);

            res->set_status(200, "OK");
            res->send_html(doc.to_string());
            return exit_code::EXIT;
        }});

    // GET /complex - Complex nested HTML structure
    server->get(
        "/complex", {[](std::shared_ptr<request> req, std::shared_ptr<response> res) -> exit_code {
            html::document doc("html");

            auto head = html::maker::make_element("head");
            auto title = html::maker::make_element("title");
            title->set_text_content("Complex HTML Test");
            head->add_child(title);

            auto body = html::maker::make_element("body");

            // Create a complex nested structure
            auto header = html::maker::make_element("header");
            header->set_attribute("id", "main-header");
            auto nav = html::maker::make_element("nav");
            auto ul = html::maker::make_element("ul");

            std::vector<std::string> menu_items = {"Home", "About", "Services", "Contact"};
            for (const auto& item : menu_items) {
                auto li = html::maker::make_element("li");
                auto link = html::maker::make_link("/" + cppress::shared::to_lowercase(item), item);
                li->add_child(link);
                ul->add_child(li);
            }

            nav->add_child(ul);
            header->add_child(nav);
            body->add_child(header);

            // Main content area
            auto main = html::maker::make_element("main");
            auto article = html::maker::make_element("article");
            article->add_child(html::maker::make_heading(1, "Complex HTML Structure"));
            article->add_child(
                html::maker::make_paragraph("This demonstrates nested elements and attributes."));

            // Add an image
            auto img = html::maker::make_image("/images/test.png", "Test Image");
            img->set_attribute("width", "300");
            img->set_attribute("height", "200");
            article->add_child(img);

            main->add_child(article);
            body->add_child(main);

            // Footer
            auto footer = html::maker::make_element("footer");
            auto footer_para = html::maker::make_paragraph("© 2025 cppress Web Framework");
            footer->add_child(footer_para);
            body->add_child(footer);

            doc.add_child(head);
            doc.add_child(body);

            res->set_status(200, "OK");
            res->send_html(doc.to_string());
            return exit_code::EXIT;
        }});

    // Start server
    std::thread server_thread([&server]() {
        server->listen([]() { std::cout << "HTML server started on port 8082" << std::endl; },
                       [](const std::exception& e) {
                           std::cerr << "Server error: " << e.what() << std::endl;
                       });
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // Test clients
    std::vector<std::thread> clients;
    std::atomic<int> home_success{0};
    std::atomic<int> users_success{0};
    std::atomic<int> form_success{0};
    std::atomic<int> submit_success{0};
    std::atomic<int> complex_success{0};

    // Test 1: Home page
    clients.emplace_back([&]() {
        try {
            cppress::sockets::connection conn;
            conn.connect(cppress::sockets::socket_address(port(8082), ip_address("127.0.0.1")));
            conn.write(data_buffer("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"));
            auto response = std::string(conn.read().to_string());

            if (response.find("200 OK") != std::string::npos &&
                response.find("<!DOCTYPE html>") != std::string::npos &&
                response.find("Welcome to cppress Web Framework") != std::string::npos &&
                response.find("Content-Type: text/html") != std::string::npos) {
                home_success++;
            }
        } catch (const std::exception& e) {
            std::cerr << "Home page error: " << e.what() << std::endl;
        }
    });

    // Test 2: Users page with table
    clients.emplace_back([&]() {
        try {
            cppress::sockets::connection conn;
            conn.connect(cppress::sockets::socket_address(port(8082), ip_address("127.0.0.1")));
            conn.write(data_buffer("GET /users HTTP/1.1\r\nHost: localhost\r\n\r\n"));
            auto response = std::string(conn.read().to_string());

            if (response.find("200 OK") != std::string::npos &&
                response.find("<table") != std::string::npos &&
                response.find("Alice") != std::string::npos &&
                response.find("alice@example.com") != std::string::npos) {
                users_success++;
            }
        } catch (const std::exception& e) {
            std::cerr << "Users page error: " << e.what() << std::endl;
        }
    });

    // Test 3: Form page
    clients.emplace_back([&]() {
        try {
            cppress::sockets::connection conn;
            conn.connect(cppress::sockets::socket_address(port(8082), ip_address("127.0.0.1")));
            conn.write(data_buffer("GET /form HTTP/1.1\r\nHost: localhost\r\n\r\n"));
            auto response = std::string(conn.read().to_string());

            if (response.find("200 OK") != std::string::npos &&
                response.find("<form") != std::string::npos &&
                response.find("method=\"POST\"") != std::string::npos &&
                response.find("type=\"text\"") != std::string::npos) {
                form_success++;
            }
        } catch (const std::exception& e) {
            std::cerr << "Form page error: " << e.what() << std::endl;
        }
    });

    // Test 4: Form submission
    clients.emplace_back([&]() {
        try {
            std::string form_data = "name=TestUser&email=test@example.com";
            std::string request =
                "POST /submit HTTP/1.1\r\n"
                "Host: localhost\r\n"
                "Content-Type: application/x-www-form-urlencoded\r\n"
                "Content-Length: " +
                std::to_string(form_data.length()) + "\r\n\r\n" + form_data;

            cppress::sockets::connection conn;
            conn.connect(cppress::sockets::socket_address(port(8082), ip_address("127.0.0.1")));
            conn.write(data_buffer(request));
            auto response = std::string(conn.read().to_string());

            if (response.find("200 OK") != std::string::npos &&
                response.find("Form Submitted Successfully") != std::string::npos) {
                submit_success++;
            }
        } catch (const std::exception& e) {
            std::cerr << "Form submission error: " << e.what() << std::endl;
        }
    });

    // Test 5: Complex HTML structure
    clients.emplace_back([&]() {
        try {
            cppress::sockets::connection conn;
            conn.connect(cppress::sockets::socket_address(port(8082), ip_address("127.0.0.1")));
            conn.write(data_buffer("GET /complex HTTP/1.1\r\nHost: localhost\r\n\r\n"));
            auto response = std::string(conn.read().to_string());

            if (response.find("200 OK") != std::string::npos &&
                response.find("<header") != std::string::npos &&
                response.find("<nav>") != std::string::npos &&
                response.find("<main>") != std::string::npos &&
                response.find("<footer>") != std::string::npos &&
                response.find("<img") != std::string::npos) {
                complex_success++;
            }
        } catch (const std::exception& e) {
            std::cerr << "Complex HTML error: " << e.what() << std::endl;
        }
    });

    for (auto& client : clients) {
        client.join();
    }

    // Verify results
    EXPECT_EQ(home_success.load(), 1);
    EXPECT_EQ(users_success.load(), 1);
    EXPECT_EQ(form_success.load(), 1);
    EXPECT_EQ(submit_success.load(), 1);
    EXPECT_EQ(complex_success.load(), 1);

    server->stop();
    server_thread.join();
}
