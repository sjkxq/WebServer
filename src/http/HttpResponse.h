#ifndef WEBSERVER_HTTP_RESPONSE_H
#define WEBSERVER_HTTP_RESPONSE_H

#include <string>
#include <map>
#include <memory>

namespace webserver {

class HttpResponse {
public:
    static std::shared_ptr<HttpResponse> create(int statusCode, 
                                              const std::string& body,
                                              const std::map<std::string, std::string>& headers = {});
    
    int getStatusCode() const;
    const std::string& getBody() const;
    const std::map<std::string, std::string>& getHeaders() const;
    
    void setStatusCode(int code);
    void setContentType(const std::string& type);
    void setBody(const std::string& body);

private:
    HttpResponse(int statusCode, 
                const std::string& body,
                const std::map<std::string, std::string>& headers);
    
    int statusCode_;
    std::string body_;
    std::map<std::string, std::string> headers_;
};

} // namespace webserver

#endif // WEBSERVER_HTTP_RESPONSE_H