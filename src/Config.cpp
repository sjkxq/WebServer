#include "Config.hpp"
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

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

// 显式实例化常用类型
template int Config::get<int>(const std::string&, const int&) const;
template void Config::set<int>(const std::string&, const int&);

template std::string Config::get<std::string>(const std::string&, const std::string&) const;
template void Config::set<std::string>(const std::string&, const std::string&);

template bool Config::get<bool>(const std::string&, const bool&) const;
template void Config::set<bool>(const std::string&, const bool&);