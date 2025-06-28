#pragma once

#include "CompressionUtil.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <vector>
#include <string>
#include <algorithm>

class HttpCompressionMiddleware {
public:
    /**
     * @brief 处理HTTP请求和响应，添加压缩支持
     * @param request HTTP请求
     * @param response HTTP响应
     */
    static void process(HttpRequest& request, HttpResponse& response);

private:
    /**
     * @brief 解析Accept-Encoding头，选择最佳压缩算法
     * @param acceptEncoding Accept-Encoding头值
     * @param preferredAlgorithms 优先考虑的算法列表
     * @return 选择的压缩算法（空字符串表示不支持或不需要压缩）
     */
    static std::string chooseCompressionAlgorithm(
        const std::string& acceptEncoding,
        const std::vector<std::string>& preferredAlgorithms);

    /**
     * @brief 压缩响应体
     * @param response HTTP响应
     * @param algorithm 压缩算法
     */
    static void compressResponse(HttpResponse& response, const std::string& algorithm);
};