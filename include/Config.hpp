#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <map>
#include <nlohmann/json.hpp>

class Config {
public:
    // 从文件加载配置
    bool loadFromFile(const std::string& filePath);
    
    // 获取配置值
    template<typename T>
    T get(const std::string& key, const T& defaultValue = T()) const;
    
    // 获取嵌套配置值（如 "server.port"）
    template<typename T>
    T getNestedValue(const std::string& path, const T& defaultValue = T()) const;
    
    // 设置配置值
    template<typename T>
    void set(const std::string& key, const T& value);
    
    // 设置嵌套配置值
    template<typename T>
    void setNestedValue(const std::string& path, const T& value);
    
private:
    // 获取嵌套JSON对象的辅助方法
    const nlohmann::json* getNestedObject(const std::string& path) const;
    nlohmann::json* getNestedObjectForUpdate(const std::string& path);
    
    nlohmann::json config_;
};

#endif // CONFIG_HPP