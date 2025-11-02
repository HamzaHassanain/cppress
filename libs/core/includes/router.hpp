#include <regex>
#include <string>

#include "../../../shared/includes/utils.hpp"
#include "types.hpp"
namespace cppress::core {

class router {
    std::vector<std::pair<route, std::vector<request_handler_t>>> routes;
};


}  // namespace cppress::core
