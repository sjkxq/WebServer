#pragma once

#include <string>
#include <map>

namespace webserver {

/**
 * @class HttpRequest
 * @brief 封装HTTP请求数据
 */
class HttpRequest {
public:
    /**
     * @brief 构造函数
     * @param method HTTP方法（GET/POST等）
     * @param path 请求路径
     * @param headers 请求头
     * @param body 请求体
     * @param queryParams 查询参数
     */
    HttpRequest(const std::string& method, 
                const std::string& path,
                const std::map<std::string, std::string>& headers,
                const std::string& body,
                const std::map<std::string, std::string>& queryParams = {});

    // 获取方法
    const std::string& getMethod() const;
    const std::string& getPath() const;
    const std::map<std::string, std::string>& getHeaders() const;
    const std::string& getBody() const;
    const std::map<std::string, std::string>& getQueryParams() const;

    /**
     * @brief 设置请求头
     * @param headers 要设置的请求头
     */
    void setHeaders(const std::map<std::string, std::string>& headers);

    /**
     * @brief 设置请求体
     * @param body 要设置的请求体
     */
    void setBody(const std::string& body);

    /**
     * @brief 获取指定头部的值
     * @param name 头部名称
     * @return 头部值，如果不存在则返回空字符串
     */
    std::string getHeader(const std::string& name) const;

    /**
     * @brief 获取指定查询参数的值
     * @param name 参数名称
     * @return 参数值，如果不存在则返回空字符串
     */
    std::string getQueryParam(const std::string& name) const;

    /**
     * @brief 检查If-Modified-Since条件请求
     * @param lastModified 资源的最后修改时间
     * @return 如果满足条件返回true
     */
    bool checkIfModifiedSince(time_t lastModified) const;

    /**
     * @brief 检查If-Unmodified-Since条件请求
     * @param lastModified 资源的最后修改时间
     * @return 如果满足条件返回true
     */
    bool checkIfUnmodifiedSince(time_t lastModified) const;

    /**
     * @brief 检查If-None-Match条件请求
     * @param etag 资源的ETag
     * @return 如果满足条件返回true
     */
    bool checkIfNoneMatch(const std::string& etag) const;

    /**
     * @brief 检查If-Match条件请求
     * @param etag 资源的ETag
     * @return 如果满足条件返回true
     */
    bool checkIfMatch(const std::string& etag) const;

private:
    std::string method_;
    std::string path_;
    std::map<std::string, std::string> headers_;
    std::string body_;
    std::map<std::string, std::string> queryParams_;
};

} // namespace webserver