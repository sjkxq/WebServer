#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <functional>
#include <vector>

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
     * @brief 原子获取整型配置值
     * @param key 配置键
     * @param defaultValue 默认值
     * @return 配置值或默认值
     * @note 线程安全
     */
    int GetInt(const std::string& key, int defaultValue = 0);
    
    /**
     * @brief 原子获取字符串配置值
     * @param key 配置键
     * @param defaultValue 默认值
     * @return 配置值或默认值
     * @note 线程安全
     */
    std::string GetString(const std::string& key, const std::string& defaultValue = "");
    
    /**
     * @brief 原子获取布尔型配置值
     * @param key 配置键
     * @param defaultValue 默认值
     * @return 配置值或默认值
     * @note 线程安全
     */
    bool GetBool(const std::string& key, bool defaultValue = false);
    
    /**
     * @brief 添加配置变更回调函数
     * @param key 配置键
     * @param callback 回调函数
     */
    void AddConfigChangeCallback(const std::string& key, std::function<void(const std::string&)> callback);

private:
    void LoadConfig();
    void WatchConfigFile();
    void NotifyConfigChange(const std::string& key);
    
    std::string configFile_;
    bool stopWatch_;
    std::thread watchThread_;
    mutable std::shared_mutex configMutex_;
    std::unordered_map<std::string, std::string> configMap_;
    std::unordered_map<std::string, std::vector<std::function<void(const std::string&)>>> callbacks_;
};

#endif // CONFIG_H
