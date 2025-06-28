#ifndef WEBSERVER_HTTP_REQUEST_H
#define WEBSERVER_HTTP_REQUEST_H

#include <string>
#include <map>

namespace webserver {

class HttpRequest {
public:
    void setHeaders(const std::map<std::string, std::string>& headers);
    void setBody(const std::string& body);
    
    const std::map<std::string, std::string>& getHeaders() const;
    const std::string& getBody() const;

private:
    std::map<std::string, std::string> headers_;
    std::string body_;
};

} // namespace webserver

#endif // WEBSERVER_HTTP_REQUEST_H