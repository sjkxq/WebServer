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
    
    // 设置配置值
    template<typename T>
    void set(const std::string& key, const T& value);
    
private:
    nlohmann::json config_;
};

#endif // CONFIG_HPP