#pragma once

#include <ctime>
#include <string>

namespace webserver {

class DateTimeUtils {
public:
    /**
     * @brief 解析HTTP日期字符串为time_t
     * @param httpDate HTTP日期字符串 (RFC 1123格式)
     * @return 解析成功返回time_t，失败返回0
     */
    static time_t parseHttpDate(const std::string& httpDate);

    /**
     * @brief 格式化time_t为HTTP日期字符串
     * @param time 时间值
     * @return HTTP日期字符串 (RFC 1123格式)
     */
    static std::string formatHttpDate(time_t time);
};

} // namespace webserver