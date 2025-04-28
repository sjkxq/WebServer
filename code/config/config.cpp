#include "config.h"
#include <algorithm>
#include <fstream>
#include <sstream>
//#include <iostream>
#include <unordered_map>

Config::Config(const char* configFile) {
    std::ifstream ifs(configFile);
    if(!ifs.is_open()) {
        throw std::runtime_error("Failed to open config file");
    }
    
    std::string line;
    while(std::getline(ifs, line)) {
        if(line.empty() || line[0] == '#') continue;
        
        size_t pos = line.find('=');
        if(pos == std::string::npos) continue;
        
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        configMap_[key] = value;
    }
    ifs.close();
}

int Config::GetInt(const std::string& key, int defaultValue) {
    auto it = configMap_.find(key);
    if(it == configMap_.end()) return defaultValue;
    
    try {
        int value = std::stoi(it->second);
        
        // 添加配置项校验逻辑
        if(key == "port" && (value < 1024 || value > 65535)) {
            throw std::runtime_error("Invalid port number, must be between 1024 and 65535");
        }
        if(key == "threadNum" && (value < 1 || value > 64)) {
            throw std::runtime_error("Invalid thread number, must be between 1 and 64");
        }
        if(key == "connPoolNum" && (value < 1 || value > 128)) {
            throw std::runtime_error("Invalid connection pool size, must be between 1 and 128");
        }
        if(key == "logQueSize" && (value < 100 || value > 10000)) {
            throw std::runtime_error("Invalid log queue size, must be between 100 and 10000");
        }
        
        return value;
    } catch(...) {
        return defaultValue;
    }
}

std::string Config::GetString(const std::string& key, const std::string& defaultValue) {
    auto it = configMap_.find(key);
    if(it == configMap_.end()) return defaultValue;
    return it->second;
}

bool Config::GetBool(const std::string& key, bool defaultValue) {
    auto it = configMap_.find(key);
    if(it == configMap_.end()) return defaultValue;
    
    std::string value = it->second;
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    
    //std::cout << "GetBool: key=" << key << ", value=" << value << std::endl;
    
    // 去除字符串首尾空格
    value.erase(0, value.find_first_not_of(" \t\n\r"));
    value.erase(value.find_last_not_of(" \t\n\r") + 1);

    // 添加布尔值校验
    if(value != "true" && value != "false" && value != "1" && value != "0") {
        std::string errMsg = "Invalid boolean value for configuration '" + key + "': '" + it->second + "', must be true/false or 1/0";
        throw std::runtime_error(errMsg);
    }
    
    return (value == "true" || value == "1");
}