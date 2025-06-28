#include "http/CompressionPolicy.hpp"
#include <algorithm>

// 初始化静态成员
CompressionPolicy::Config CompressionPolicy::defaultConfig = {
    true,    // enabled
    1024,    // minSizeToCompress
    {"gzip", "deflate"} // preferredAlgorithms
};

std::unordered_map<std::string, CompressionPolicy::Config> CompressionPolicy::contentTypeConfigs;

void CompressionPolicy::setDefaultConfig(const Config& config) {
    defaultConfig = config;
}

void CompressionPolicy::setContentTypeConfig(const std::string& contentType, const Config& config) {
    contentTypeConfigs[contentType] = config;
}

CompressionPolicy::Config CompressionPolicy::getConfigForContentType(const std::string& contentType) {
    // 查找完全匹配
    auto exactMatch = contentTypeConfigs.find(contentType);
    if (exactMatch != contentTypeConfigs.end()) {
        return exactMatch->second;
    }

    // 查找通配符匹配（如 text/*）
    size_t slashPos = contentType.find('/');
    if (slashPos != std::string::npos) {
        std::string wildcard = contentType.substr(0, slashPos) + "/*";
        auto wildcardMatch = contentTypeConfigs.find(wildcard);
        if (wildcardMatch != contentTypeConfigs.end()) {
            return wildcardMatch->second;
        }
    }

    // 返回默认配置
    return defaultConfig;
}