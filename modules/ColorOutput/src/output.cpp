#include "webserver/color/output.hpp"
#include <unordered_map>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#endif

namespace webserver::color {

namespace {
    // ANSI颜色代码映射
    const std::unordered_map<Color, int> ANSI_COLOR_CODES = {
        {Color::BLACK, 30},
        {Color::RED, 31},
        {Color::GREEN, 32},
        {Color::YELLOW, 33},
        {Color::BLUE, 34},
        {Color::MAGENTA, 35},
        {Color::CYAN, 36},
        {Color::WHITE, 37},
        {Color::DEFAULT, 39}
    };

    const std::unordered_map<Background, int> ANSI_BG_CODES = {
        {Background::BLACK, 40},
        {Background::RED, 41},
        {Background::GREEN, 42},
        {Background::YELLOW, 43},
        {Background::BLUE, 44},
        {Background::MAGENTA, 45},
        {Background::CYAN, 46},
        {Background::WHITE, 47},
        {Background::DEFAULT, 49}
    };

    const std::unordered_map<Style, int> ANSI_STYLE_CODES = {
        {Style::RESET, 0},
        {Style::BOLD, 1},
        {Style::DIM, 2},
        {Style::ITALIC, 3},
        {Style::UNDERLINE, 4},
        {Style::BLINK, 5},
        {Style::REVERSE, 7},
        {Style::HIDDEN, 8}
    };

#ifdef _WIN32
    // Windows控制台颜色代码映射
    const std::unordered_map<Color, WORD> WIN_COLOR_CODES = {
        {Color::BLACK, 0},
        {Color::RED, FOREGROUND_RED},
        {Color::GREEN, FOREGROUND_GREEN},
        {Color::YELLOW, FOREGROUND_RED | FOREGROUND_GREEN},
        {Color::BLUE, FOREGROUND_BLUE},
        {Color::MAGENTA, FOREGROUND_RED | FOREGROUND_BLUE},
        {Color::CYAN, FOREGROUND_GREEN | FOREGROUND_BLUE},
        {Color::WHITE, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE},
        {Color::DEFAULT, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE}
    };

    const std::unordered_map<Background, WORD> WIN_BG_CODES = {
        {Background::BLACK, 0},
        {Background::RED, BACKGROUND_RED},
        {Background::GREEN, BACKGROUND_GREEN},
        {Background::YELLOW, BACKGROUND_RED | BACKGROUND_GREEN},
        {Background::BLUE, BACKGROUND_BLUE},
        {Background::MAGENTA, BACKGROUND_RED | BACKGROUND_BLUE},
        {Background::CYAN, BACKGROUND_GREEN | BACKGROUND_BLUE},
        {Background::WHITE, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE},
        {Background::DEFAULT, 0}
    };
#endif
}

// ANSI颜色处理器实现
std::string AnsiColorHandler::applyColor(Color color) const {
    return "\033[" + std::to_string(ANSI_COLOR_CODES.at(color)) + "m";
}

std::string AnsiColorHandler::applyBackground(Background bg) const {
    return "\033[" + std::to_string(ANSI_BG_CODES.at(bg)) + "m";
}

std::string AnsiColorHandler::applyStyle(Style style) const {
    return "\033[" + std::to_string(ANSI_STYLE_CODES.at(style)) + "m";
}

bool AnsiColorHandler::supportsColor() const {
    // 检查是否在终端环境中
    const char* term = std::getenv("TERM");
    if (term == nullptr) return false;
    
    // 检查是否支持颜色的终端类型
    std::string termStr(term);
    return termStr != "dumb" && termStr != "unknown";
}

#ifdef _WIN32
// Windows颜色处理器实现
std::string WindowsColorHandler::applyColor(Color color) const {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        WORD newAttributes = (csbi.wAttributes & 0xFFF0) | WIN_COLOR_CODES.at(color);
        SetConsoleTextAttribute(hConsole, newAttributes);
    }
    return "";
}

std::string WindowsColorHandler::applyBackground(Background bg) const {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        WORD newAttributes = (csbi.wAttributes & 0xFF0F) | WIN_BG_CODES.at(bg);
        SetConsoleTextAttribute(hConsole, newAttributes);
    }
    return "";
}

std::string WindowsColorHandler::applyStyle(Style style) const {
    // Windows控制台对样式支持有限，这里只实现基本的样式
    if (style == Style::RESET) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole != INVALID_HANDLE_VALUE) {
            SetConsoleTextAttribute(hConsole, WIN_COLOR_CODES.at(Color::DEFAULT));
        }
    }
    return "";
}

bool WindowsColorHandler::supportsColor() const {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    return hConsole != INVALID_HANDLE_VALUE;
}
#endif

// ColorOutput单例实现
ColorOutput& ColorOutput::getInstance() {
    static ColorOutput instance;
    return instance;
}

ColorOutput::ColorOutput() {
    autoDetectHandler();
}

void ColorOutput::setEnabled(bool enabled) {
    m_enabled = enabled;
}

bool ColorOutput::isEnabled() const {
    return m_enabled;
}

const ColorHandler* ColorOutput::getHandler() const {
    return m_handler.get();
}

void ColorOutput::setHandler(std::unique_ptr<ColorHandler> handler) {
    m_handler = std::move(handler);
}

void ColorOutput::autoDetectHandler() {
#ifdef _WIN32
    auto handler = std::make_unique<WindowsColorHandler>();
    if (handler->supportsColor()) {
        m_handler = std::move(handler);
        return;
    }
#endif

    auto handler = std::make_unique<AnsiColorHandler>();
    if (handler->supportsColor()) {
        m_handler = std::move(handler);
        return;
    }

    m_handler = std::make_unique<NullColorHandler>();
}

// ColorFormatter实现
std::string ColorFormatter::colorize(const std::string& text, Color color) {
    auto& output = ColorOutput::getInstance();
    if (!output.isEnabled() || !output.getHandler()) {
        return text;
    }

    const auto* handler = output.getHandler();
    return handler->applyColor(color) + text + handler->applyStyle(Style::RESET);
}

std::string ColorFormatter::colorize(const std::string& text, Color color, Background bg) {
    auto& output = ColorOutput::getInstance();
    if (!output.isEnabled() || !output.getHandler()) {
        return text;
    }

    const auto* handler = output.getHandler();
    return handler->applyColor(color) + handler->applyBackground(bg) + text + 
           handler->applyStyle(Style::RESET);
}

std::string ColorFormatter::colorize(const std::string& text, Color color, Background bg, Style style) {
    auto& output = ColorOutput::getInstance();
    if (!output.isEnabled() || !output.getHandler()) {
        return text;
    }

    const auto* handler = output.getHandler();
    return handler->applyColor(color) + handler->applyBackground(bg) + 
           handler->applyStyle(style) + text + handler->applyStyle(Style::RESET);
}

// 流操作符实现
std::ostream& operator<<(std::ostream& os, Color color) {
    auto& output = ColorOutput::getInstance();
    if (output.isEnabled() && output.getHandler()) {
        os << output.getHandler()->applyColor(color);
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, Background bg) {
    auto& output = ColorOutput::getInstance();
    if (output.isEnabled() && output.getHandler()) {
        os << output.getHandler()->applyBackground(bg);
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, Style style) {
    auto& output = ColorOutput::getInstance();
    if (output.isEnabled() && output.getHandler()) {
        os << output.getHandler()->applyStyle(style);
    }
    return os;
}

} // namespace webserver::color