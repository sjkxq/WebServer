/**
 * @file Logger.hpp
 * @brief 日志记录器类的声明
 */

#ifndef WEBSERVER_LOGGER_HPP
#define WEBSERVER_LOGGER_HPP

#include <string>
#include <fstream>
#include <iostream>
#include <mutex>

namespace webserver {

/**
 * @class Logger
 * @brief 单例日志记录器类，提供线程安全的日志记录功能
 */
class Logger {
public:
    /**
     * @brief 日志级别枚举
     */
    enum class Level {
        TRACE,      ///< 跟踪信息
        DEBUG_LEVEL, ///< 调试信息（避免与DEBUG宏冲突，使用DEBUG_LEVEL）
        INFO,       ///< 一般信息
        WARNING,    ///< 警告信息
        ERROR,      ///< 错误信息
        FATAL       ///< 致命错误
    };

    /**
     * @brief 获取Logger单例实例
     * @return Logger单例的引用
     */
    static Logger& getInstance();

    /**
     * @brief 设置日志输出文件
     * @param filename 日志文件路径
     */
    void setLogFile(const std::string& filename);

    /**
     * @brief 设置日志输出流
     * @param stream 输出流引用
     */
    void setStream(std::ostream& stream);

    /**
     * @brief 设置是否输出到控制台
     * @param enable true表示输出到控制台，false表示不输出
     */
    void setConsoleOutput(bool enable);

    /**
     * @brief 记录日志
     * @param level 日志级别
     * @param message 日志消息
     */
    void log(Level level, const std::string& message);

    /**
     * @brief 设置最低日志级别
     * @param level 日志级别
     */
    void setLogLevel(Level level) { minLevel = level; }

    // 禁止拷贝构造和赋值操作
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    // 私有构造函数，确保单例模式
    Logger();
    ~Logger();

    // 辅助方法
    std::string getLevelString(Level level) const;
    std::string getCurrentTimestamp() const;

    // 成员变量
    Level minLevel;
    std::ostream* outputStream;
    std::ofstream fileStream;
    bool consoleOutput;
    std::mutex logMutex;
};

// 日志宏，简化日志记录
#define LOG_TRACE(msg) webserver::Logger::getInstance().log(webserver::Logger::Level::TRACE, msg)
#define LOG_DEBUG(msg) webserver::Logger::getInstance().log(webserver::Logger::Level::DEBUG_LEVEL, msg)
#define LOG_INFO(msg) webserver::Logger::getInstance().log(webserver::Logger::Level::INFO, msg)
#define LOG_WARNING(msg) webserver::Logger::getInstance().log(webserver::Logger::Level::WARNING, msg)
#define LOG_ERROR(msg) webserver::Logger::getInstance().log(webserver::Logger::Level::ERROR, msg)
#define LOG_FATAL(msg) webserver::Logger::getInstance().log(webserver::Logger::Level::FATAL, msg)

} // namespace webserver

#endif // WEBSERVER_LOGGER_HPP