#ifndef WEBSERVER_CONFIG_HPP
#define WEBSERVER_CONFIG_HPP

#include <string>
#include <map>
#include <nlohmann/json.hpp>

namespace webserver {

/**
 * @class Config
 * @brief 处理服务器配置的类，支持JSON格式的配置文件
 */
class Config {
public:
    /**
     * @brief 从文件加载配置
     * @param filePath 配置文件路径
     * @return 加载成功返回true，否则返回false
     */
    bool loadFromFile(const std::string& filePath);
    
    /**
     * @brief 获取配置值
     * @param key 配置键
     * @param defaultValue 默认值
     * @return 配置值，如果不存在则返回默认值
     */
    template<typename T>
    T get(const std::string& key, const T& defaultValue = T()) const;
    
    /**
     * @brief 获取嵌套配置值（如 "server.port"）
     * @param path 嵌套路径（使用点号分隔）
     * @param defaultValue 默认值
     * @return 配置值，如果不存在则返回默认值
     */
    template<typename T>
    T getNestedValue(const std::string& path, const T& defaultValue = T()) const;
    
    /**
     * @brief 设置配置值
     * @param key 配置键
     * @param value 要设置的值
     */
    template<typename T>
    void set(const std::string& key, const T& value);
    
    /**
     * @brief 设置嵌套配置值
     * @param path 嵌套路径（使用点号分隔）
     * @param value 要设置的值
     */
    template<typename T>
    void setNestedValue(const std::string& path, const T& value);
    
private:
    /**
     * @brief 获取嵌套JSON对象的辅助方法
     * @param path 嵌套路径
     * @return 指向嵌套JSON对象的指针，如果路径无效则返回nullptr
     */
    const nlohmann::json* getNestedObject(const std::string& path) const;
    
    /**
     * @brief 获取用于更新的嵌套JSON对象
     * @param path 嵌套路径
     * @return 指向嵌套JSON对象的指针，如果路径无效则返回nullptr
     */
    nlohmann::json* getNestedObjectForUpdate(const std::string& path);
    
    nlohmann::json config_;  // JSON格式的配置数据
};

} // namespace webserver

#endif // WEBSERVER_CONFIG_HPP