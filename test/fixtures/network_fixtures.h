#ifndef WEBSERVER_TEST_FIXTURES_NETWORK_FIXTURES_H_
#define WEBSERVER_TEST_FIXTURES_NETWORK_FIXTURES_H_

#include "base_fixtures.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include <thread>
#include <future>

namespace WebServer {
namespace test {

// 基础网络测试夹具
class NetworkFixture : public ServerTestFixture {
protected:
    void SetUp() override {
        ServerTestFixture::SetUp();
        // 创建服务器socket
        server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
        ASSERT_NE(server_socket_, -1) << "Failed to create server socket";

        // 设置socket选项
        int opt = 1;
        ASSERT_NE(setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                            &opt, sizeof(opt)), -1) << "Failed to set socket options";

        // 绑定到随机端口
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = 0;  // 让系统分配随机端口

        ASSERT_NE(bind(server_socket_, (struct sockaddr*)&address,
                      sizeof(address)), -1) << "Failed to bind socket";

        // 获取分配的端口号
        socklen_t len = sizeof(address);
        ASSERT_NE(getsockname(server_socket_, (struct sockaddr*)&address,
                             &len), -1) << "Failed to get socket name";
        
        port_ = ntohs(address.sin_port);
    }

    void TearDown() override {
        if (server_socket_ != -1) {
            close(server_socket_);
        }
        ServerTestFixture::TearDown();
    }

    // 启动监听
    void StartListening(int backlog = 5) {
        ASSERT_NE(listen(server_socket_, backlog), -1) << "Failed to listen on socket";
    }

    // 创建客户端连接
    int CreateClientConnection() {
        int client_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (client_sock == -1) {
            return -1;
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port_);
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(client_sock, (struct sockaddr*)&server_addr,
                   sizeof(server_addr)) == -1) {
            close(client_sock);
            return -1;
        }

        return client_sock;
    }

    // 接受连接
    int AcceptConnection() {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        return accept(server_socket_, (struct sockaddr*)&client_addr, &client_len);
    }

    // 设置非阻塞模式
    void SetNonBlocking(int sock) {
        int flags = fcntl(sock, F_GETFL, 0);
        ASSERT_NE(flags, -1) << "Failed to get socket flags";
        ASSERT_NE(fcntl(sock, F_SETFL, flags | O_NONBLOCK), -1)
            << "Failed to set non-blocking mode";
    }

    // 发送数据
    bool SendData(int sock, const std::string& data) {
        ssize_t total_sent = 0;
        while (total_sent < static_cast<ssize_t>(data.length())) {
            ssize_t sent = send(sock, data.c_str() + total_sent,
                              data.length() - total_sent, 0);
            if (sent == -1) {
                return false;
            }
            total_sent += sent;
        }
        return true;
    }

    // 接收数据
    std::string ReceiveData(int sock, size_t max_size = 1024) {
        std::vector<char> buffer(max_size);
        ssize_t received = recv(sock, buffer.data(), buffer.size(), 0);
        if (received <= 0) {
            return "";
        }
        return std::string(buffer.data(), received);
    }

    // 测试并发连接
    void TestConcurrentConnections(size_t connection_count) {
        StartListening(connection_count);

        std::vector<std::future<bool>> futures;
        for (size_t i = 0; i < connection_count; ++i) {
            futures.push_back(std::async(std::launch::async, [this]() {
                int client_sock = CreateClientConnection();
                if (client_sock == -1) {
                    return false;
                }
                close(client_sock);
                return true;
            }));
        }

        // 等待所有连接完成
        for (auto& future : futures) {
            EXPECT_TRUE(future.get());
        }
    }

protected:
    int server_socket_{-1};
    uint16_t port_{0};
};

// HTTP测试夹具
class HttpFixture : public NetworkFixture {
protected:
    // 构造HTTP请求
    std::string BuildHttpRequest(const std::string& method,
                               const std::string& path,
                               const std::string& body = "",
                               const std::vector<std::pair<std::string, std::string>>& headers = {}) {
        std::stringstream request;
        request << method << " " << path << " HTTP/1.1\r\n";
        request << "Host: localhost:" << port_ << "\r\n";
        
        for (const auto& header : headers) {
            request << header.first << ": " << header.second << "\r\n";
        }
        
        if (!body.empty()) {
            request << "Content-Length: " << body.length() << "\r\n";
        }
        
        request << "\r\n";
        
        if (!body.empty()) {
            request << body;
        }
        
        return request.str();
    }

    // 解析HTTP响应
    struct HttpResponse {
        int status_code;
        std::string status_message;
        std::vector<std::pair<std::string, std::string>> headers;
        std::string body;
    };

    HttpResponse ParseHttpResponse(const std::string& response_str) {
        HttpResponse response;
        std::istringstream response_stream(response_str);
        std::string line;

        // 解析状态行
        std::getline(response_stream, line);
        std::smatch matches;
        std::regex status_line_regex(R"(HTTP/\d\.\d (\d+) (.*))");
        if (std::regex_match(line, matches, status_line_regex)) {
            response.status_code = std::stoi(matches[1]);
            response.status_message = matches[2];
        }

        // 解析头部
        while (std::getline(response_stream, line) && line != "\r") {
            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                std::string name = line.substr(0, colon_pos);
                std::string value = line.substr(colon_pos + 2);  // Skip ": "
                if (!value.empty() && value.back() == '\r') {
                    value.pop_back();
                }
                response.headers.emplace_back(name, value);
            }
        }

        // 读取响应体
        std::stringstream body_stream;
        body_stream << response_stream.rdbuf();
        response.body = body_stream.str();

        return response;
    }

    // 发送HTTP请求并获取响应
    HttpResponse SendHttpRequest(const std::string& method,
                               const std::string& path,
                               const std::string& body = "",
                               const std::vector<std::pair<std::string, std::string>>& headers = {}) {
        int client_sock = CreateClientConnection();
        EXPECT_NE(client_sock, -1) << "Failed to create client connection";

        std::string request = BuildHttpRequest(method, path, body, headers);
        EXPECT_TRUE(SendData(client_sock, request)) << "Failed to send request";

        std::string response_str = ReceiveData(client_sock, 4096);
        close(client_sock);

        return ParseHttpResponse(response_str);
    }

    // 测试基本的HTTP方法
    void TestHttpMethod(const std::string& method,
                       const std::string& path,
                       int expected_status = 200) {
        auto response = SendHttpRequest(method, path);
        EXPECT_EQ(response.status_code, expected_status);
    }
};

// 参数化HTTP测试夹具
class HttpParameterizedFixture :
    public HttpFixture,
    public ::testing::WithParamInterface<std::string> {
protected:
    // 获取当前测试的HTTP方法
    std::string GetHttpMethod() const {
        return GetParam();
    }
};

// 定义常用的HTTP方法参数
const std::vector<std::string> HTTP_METHODS = {
    "GET",
    "POST",
    "PUT",
    "DELETE",
    "HEAD",
    "OPTIONS"
};

} // namespace test
} // namespace WebServer

#endif // WEBSERVER_TEST_FIXTURES_NETWORK_FIXTURES_H_