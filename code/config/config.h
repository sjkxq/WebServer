#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <unordered_map>
#include <thread>
#include <mutex>

/**
 * @brief 配置类，用于读取和解析配置文件
 */
class Config {
public:
    /**
     * @brief 构造函数
     * @param configFile 配置文件路径
     */
    explicit Config(const char* configFile);
    
    ~Config();
    
    /**
     * @brief 获取整型配置值
     * @param key 配置键
     * @param defaultValue 默认值
     * @return 配置值或默认值
     */
    int GetInt(const std::string& key, int defaultValue = 0);
    
    /**
     * @brief 获取字符串配置值
     * @param key 配置键
     * @param defaultValue 默认值
     * @return 配置值或默认值
     */
    std::string GetString(const std::string& key, const std::string& defaultValue = "");
    
    /**
     * @brief 获取布尔型配置值
     * @param key 配置键
     * @param defaultValue 默认值
     * @return 配置值或默认值
     */
    bool GetBool(const std::string& key, bool defaultValue = false);

private:
    void LoadConfig();
    void WatchConfigFile();
    
    std::string configFile_;
    bool stopWatch_;
    std::thread watchThread_;
    std::mutex configMutex_;
    std::unordered_map<std::string, std::string> configMap_;
};

#endif // CONFIG_H
