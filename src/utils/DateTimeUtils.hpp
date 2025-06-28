#ifndef WEBSERVER_DATETIMEUTILS_HPP
#define WEBSERVER_DATETIMEUTILS_HPP

#include <string>
#include <ctime>

namespace webserver {

class DateTimeUtils {
public:
    static time_t parseHttpDate(const std::string& httpDate);
    static std::string formatHttpDate(time_t time);
};

} // namespace webserver

#endif // WEBSERVER_DATETIMEUTILS_HPP