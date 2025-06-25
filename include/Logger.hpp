#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>
#include <iostream>
#include <mutex>
#include <memory>

// 测试类的前向声明
class LoggerTest;

class Logger {
    friend class LoggerTest;
public:
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void setLogFile(const std::string& filename);
    void setStream(std::ostream& stream);
    void log(Level level, const std::string& message);
    void setLogLevel(Level level) { minLevel = level; }

    // 删除拷贝构造函数和赋值运算符
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger() : minLevel(Level::INFO) {}
    std::string getLevelString(Level level) const;
    std::string getCurrentTimestamp() const;

    Level minLevel;
    std::mutex logMutex;
    std::ostream* outputStream = &std::cout;
    std::unique_ptr<std::ofstream> logFile;
};

// 便捷宏定义
#define LOG_DEBUG(msg) Logger::getInstance().log(Logger::Level::DEBUG, msg)
#define LOG_INFO(msg) Logger::getInstance().log(Logger::Level::INFO, msg)
#define LOG_WARNING(msg) Logger::getInstance().log(Logger::Level::WARNING, msg)
#define LOG_ERROR(msg) Logger::getInstance().log(Logger::Level::ERROR, msg)

#endif // LOGGER_HPP