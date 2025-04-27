/*
 * @Author       : mark
 * @Date         : 2023-11-15
 * @copyleft Apache 2.0
 */

#include "config.h"
#include <fstream>
#include <sstream>
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
        return std::stoi(it->second);
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
    return (value == "true" || value == "1");
}