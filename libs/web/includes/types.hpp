#pragma once
#include <functional>
#include <initializer_list>
#include <memory>

#include "exceptions.hpp"
#include "http/includes.hpp"
#include "request.hpp"
#include "response.hpp"

namespace cppress::web {
enum class exit_code { EXIT = 1, CONTINUE = 0, _ERROR = -1 };
using http_request_callback_t =
    std::function<void(cppress::http::http_request&, cppress::http::http_response&)>;
using listen_callback_t = std::function<void()>;
using error_callback_t = std::function<void(const std::exception&)>;

template <typename T = request, typename G = response>
using unhandled_exception_callback_t =
    std::function<void(std::shared_ptr<T>, std::shared_ptr<G>, const exception&)>;

template <typename T = request, typename G = response>
using request_handler_t = std::function<exit_code(std::shared_ptr<T>, std::shared_ptr<G>)>;

};  // namespace cppress::web