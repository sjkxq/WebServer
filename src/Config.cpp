#include "Config.hpp"
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <vector>

using json = nlohmann::json;

namespace webserver {

// 辅助函数：分割路径字符串
std::vector<std::string> splitPath(const std::string& path) {
    std::vector<std::string> parts;
    std::stringstream ss(path);
    std::string part;
    
    while (std::getline(ss, part, '.')) {
        if (!part.empty()) {
            parts.push_back(part);
        }
    }
    
    return parts;
}

bool Config::loadFromFile(const std::string& filePath) {
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return false;
        }
        file >> config_;
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

template<typename T>
T Config::get(const std::string& key, const T& defaultValue) const {
    try {
        return config_.value(key, defaultValue);
    } catch (...) {
        return defaultValue;
    }
}

template<typename T>
void Config::set(const std::string& key, const T& value) {
    config_[key] = value;
}

// 获取嵌套JSON对象（只读）
const nlohmann::json* Config::getNestedObject(const std::string& path) const {
    std::vector<std::string> parts = splitPath(path);
    const nlohmann::json* current = &config_;
    
    for (const auto& part : parts) {
        if (!current->is_object() || !current->contains(part)) {
            return nullptr;
        }
        current = &(*current)[part];
    }
    
    return current;
}

// 获取嵌套JSON对象（可修改）
nlohmann::json* Config::getNestedObjectForUpdate(const std::string& path) {
    std::vector<std::string> parts = splitPath(path);
    nlohmann::json* current = &config_;
    
    for (size_t i = 0; i < parts.size(); ++i) {
        const auto& part = parts[i];
        if (i == parts.size() - 1) {
            // 最后一部分，直接返回父对象
            return current;
        }
        
        // 如果不存在或不是对象，创建一个新对象
        if (!current->contains(part) || !(*current)[part].is_object()) {
            (*current)[part] = json::object();
        }
        
        current = &(*current)[part];
    }
    
    return current;
}

// 获取嵌套配置值
template<typename T>
T Config::getNestedValue(const std::string& path, const T& defaultValue) const {
    std::vector<std::string> parts = splitPath(path);
    if (parts.empty()) {
        return defaultValue;
    }
    
    const nlohmann::json* obj = getNestedObject(path.substr(0, path.rfind('.')));
    if (!obj || !obj->contains(parts.back())) {
        return defaultValue;
    }
    
    try {
        return (*obj)[parts.back()].get<T>();
    } catch (...) {
        return defaultValue;
    }
}

// 设置嵌套配置值
template<typename T>
void Config::setNestedValue(const std::string& path, const T& value) {
    std::vector<std::string> parts = splitPath(path);
    if (parts.empty()) {
        return;
    }
    
    nlohmann::json* obj = getNestedObjectForUpdate(path.substr(0, path.rfind('.')));
    if (obj) {
        (*obj)[parts.back()] = value;
    }
}

// 显式实例化常用类型
template int Config::get<int>(const std::string&, const int&) const;
template void Config::set<int>(const std::string&, const int&);

template std::string Config::get<std::string>(const std::string&, const std::string&) const;
template void Config::set<std::string>(const std::string&, const std::string&);

template bool Config::get<bool>(const std::string&, const bool&) const;
template void Config::set<bool>(const std::string&, const bool&);

// 显式实例化嵌套值访问方法
template int Config::getNestedValue<int>(const std::string&, const int&) const;
template void Config::setNestedValue<int>(const std::string&, const int&);

template std::string Config::getNestedValue<std::string>(const std::string&, const std::string&) const;
template void Config::setNestedValue<std::string>(const std::string&, const std::string&);

template bool Config::getNestedValue<bool>(const std::string&, const bool&) const;
template void Config::setNestedValue<bool>(const std::string&, const bool&);

} // namespace webserver