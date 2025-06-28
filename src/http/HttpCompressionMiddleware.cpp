#include "http/HttpCompressionMiddleware.hpp"
#include "http/CompressionPolicy.hpp"
#include <sstream>
#include <algorithm>

void HttpCompressionMiddleware::process(HttpRequest& request, HttpResponse& response) {
    // 检查是否应该压缩响应
    if (response.getBody().empty() || 
        response.hasHeader("Content-Encoding") ||
        !response.shouldCompress()) {
        return;
    }

    // 获取内容类型和策略
    std::string contentType = response.getHeader("Content-Type");
    auto policy = CompressionPolicy::getConfigForContentType(contentType);
    
    // 检查是否启用压缩和最小大小
    if (!policy.enabled || response.getBody().size() < policy.minSizeToCompress) {
        return;
    }

    // 获取Accept-Encoding头
    std::string acceptEncoding = request.getHeader("Accept-Encoding");
    if (acceptEncoding.empty()) {
        return; // 客户端不支持压缩
    }

    // 选择最佳压缩算法（使用策略中的优先级）
    std::string algorithm = chooseCompressionAlgorithm(acceptEncoding, policy.preferredAlgorithms);
    if (algorithm.empty()) {
        return; // 没有共同支持的算法
    }

    // 压缩响应体
    compressResponse(response, algorithm);
}

std::string HttpCompressionMiddleware::chooseCompressionAlgorithm(
    const std::string& acceptEncoding,
    const std::vector<std::string>& preferredAlgorithms) {
    
    // 转换为小写以便比较
    std::string lowerAccept = acceptEncoding;
    std::transform(lowerAccept.begin(), lowerAccept.end(), lowerAccept.begin(), ::tolower);

    // 使用策略提供的算法优先级
    if (!preferredAlgorithms.empty()) {
        for (const auto& encoding : preferredAlgorithms) {
            std::string lowerEncoding = encoding;
            std::transform(lowerEncoding.begin(), lowerEncoding.end(), lowerEncoding.begin(), ::tolower);
            if (lowerAccept.find(lowerEncoding) != std::string::npos) {
                return lowerEncoding;
            }
        }
    }

    // 默认回退到gzip和deflate
    if (lowerAccept.find("gzip") != std::string::npos) {
        return "gzip";
    }
    if (lowerAccept.find("deflate") != std::string::npos) {
        return "deflate";
    }

    return ""; // 没有匹配的编码
}

void HttpCompressionMiddleware::compressResponse(HttpResponse& response, const std::string& algorithm) {
    try {
        std::vector<uint8_t> compressedBody;
        const auto& originalBody = response.getBody();

        if (algorithm == "gzip" || algorithm == "deflate") {
            // 使用我们之前实现的CompressionUtil
            compressedBody = CompressionUtil::compress(originalBody);
        }

        if (!compressedBody.empty()) {
            response.setBody(compressedBody);
            response.setHeader("Content-Encoding", algorithm);
            response.setHeader("Vary", "Accept-Encoding"); // 告知缓存系统
        }
    } catch (const std::exception& e) {
        // 压缩失败，保持原始响应
        response.removeHeader("Content-Encoding");
    }
}