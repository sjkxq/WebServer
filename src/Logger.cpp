#include "Logger.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

void Logger::setLogFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(logMutex);
    logFile = std::make_unique<std::ofstream>(filename, std::ios::app);
    if (!logFile->is_open()) {
        std::cerr << "无法打开日志文件: " << filename << std::endl;
        logFile.reset();
    }
}

void Logger::setStream(std::ostream& stream) {
    std::lock_guard<std::mutex> lock(logMutex);
    outputStream = &stream;
}

void Logger::log(Level level, const std::string& message) {
    if (level < minLevel) {
        return;
    }

    std::string timestamp = getCurrentTimestamp();
    std::string levelStr = getLevelString(level);
    std::string formattedMessage = timestamp + " [" + levelStr + "] " + message;

    std::lock_guard<std::mutex> lock(logMutex);
    
    // 输出到设置的流
    *outputStream << formattedMessage << std::endl;
    
    // 输出到文件（如果已设置）
    if (logFile && logFile->is_open()) {
        *logFile << formattedMessage << std::endl;
        logFile->flush();
    }
}

std::string Logger::getLevelString(Level level) const {
    switch (level) {
        case Level::INFO:
            return "INFO";
        case Level::WARNING:
            return "WARNING";
        case Level::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

std::string Logger::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}