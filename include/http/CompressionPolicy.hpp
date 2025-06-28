#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class CompressionPolicy {
public:
    // 压缩策略配置
    struct Config {
        bool enabled = true;             // 是否启用压缩
        size_t minSizeToCompress = 1024; // 最小压缩大小（字节）
        std::vector<std::string> preferredAlgorithms; // 优先算法列表
    };

    /**
     * @brief 设置默认压缩配置
     * @param config 默认配置
     */
    static void setDefaultConfig(const Config& config);

    /**
     * @brief 设置特定内容类型的压缩配置
     * @param contentType 内容类型（如"text/html"）
     * @param config 配置
     */
    static void setContentTypeConfig(const std::string& contentType, const Config& config);

    /**
     * @brief 获取内容类型的压缩配置
     * @param contentType 内容类型
     * @return 压缩配置
     */
    static Config getConfigForContentType(const std::string& contentType);

private:
    static Config defaultConfig;
    static std::unordered_map<std::string, Config> contentTypeConfigs;
};