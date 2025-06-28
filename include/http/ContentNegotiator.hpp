#pragma once

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <vector>
#include <string>
#include <map>

class ContentNegotiator {
public:
    /**
     * @brief 协商最佳响应表示形式
     * @param request HTTP请求
     * @param possibleResponses 可能的响应表示形式
     * @return 最佳匹配响应
     */
    static HttpResponse negotiate(const HttpRequest& request, 
                                const std::vector<HttpResponse>& possibleResponses);

private:
    /**
     * @brief 解析带质量值的头部字段
     * @param headerValue 头部值
     * @return 排序后的偏好列表(带质量值)
     */
    static std::vector<std::pair<std::string, float>> parseQualityValues(const std::string& headerValue);

    /**
     * @brief 选择最佳媒体类型
     */
    static std::string negotiateContentType(
        const std::string& acceptHeader,
        const std::vector<std::string>& availableTypes);

    /**
     * @brief 选择最佳语言
     */
    static std::string negotiateLanguage(
        const std::string& acceptLanguageHeader,
        const std::vector<std::string>& availableLangs);

    /**
     * @brief 选择最佳编码
     */
    static std::string negotiateEncoding(
        const std::string& acceptEncodingHeader,
        const std::vector<std::string>& availableEncodings);

    /**
     * @brief 选择最佳字符集
     */
    static std::string negotiateCharset(
        const std::string& acceptCharsetHeader,
        const std::vector<std::string>& availableCharsets);
};