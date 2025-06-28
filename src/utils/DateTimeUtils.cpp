#include "utils/DateTimeUtils.hpp"
#include <ctime>
#include <iomanip>
#include <sstream>

namespace webserver {

time_t DateTimeUtils::parseHttpDate(const std::string& httpDate) {
    std::tm tm = {};
    std::istringstream ss(httpDate);
    
    // 尝试解析RFC 1123格式 (e.g. "Sun, 06 Nov 1994 08:49:37 GMT")
    ss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
    if (ss.fail()) {
        return 0;
    }
    
    return std::mktime(&tm);
}

std::string DateTimeUtils::formatHttpDate(time_t time) {
    std::tm* tm = std::gmtime(&time);
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", tm);
    return std::string(buffer);
}

} // namespace webserver