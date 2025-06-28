#pragma once

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "ContentNegotiator.hpp"
#include <vector>

class HttpProcessor {
public:
    /**
     * @brief 处理HTTP请求
     * @param request HTTP请求
     * @param possibleResponses 可能的响应表示形式(用于内容协商)
     * @return HTTP响应
     */
    HttpResponse process(const HttpRequest& request, 
                        const std::vector<HttpResponse>& possibleResponses = {});

protected:
    /**
     * @brief 准备可能的响应表示形式
     * @param request HTTP请求
     * @return 可能的响应列表
     */
    virtual std::vector<HttpResponse> prepareResponses(const HttpRequest& request);
    
    /**
     * @brief 创建错误响应
     * @param statusCode HTTP状态码
     * @param message 错误消息
     * @return HTTP错误响应
     */
    virtual HttpResponse createErrorResponse(int statusCode, const std::string& message);
};