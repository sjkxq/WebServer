#pragma once

#include <string>
#include <ostream>

namespace webserver::color {

/**
 * @brief 文本颜色枚举
 */
enum class Color {
    BLACK = 30,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
    DEFAULT = 39
};

/**
 * @brief 背景颜色枚举
 */
enum class Background {
    BLACK = 40,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
    DEFAULT = 49
};

/**
 * @brief 文本样式枚举
 */
enum class Style {
    RESET = 0,
    BOLD = 1,
    DIM = 2,
    ITALIC = 3,
    UNDERLINE = 4,
    BLINK = 5,
    REVERSE = 7,
    HIDDEN = 8
};

/**
 * @brief 颜色输出流操作符
 * @param os 输出流
 * @param color 颜色值
 * @return 输出流引用
 */
std::ostream& operator<<(std::ostream& os, Color color);

/**
 * @brief 背景色输出流操作符
 * @param os 输出流
 * @param bg 背景色值
 * @return 输出流引用
 */
std::ostream& operator<<(std::ostream& os, Background bg);

/**
 * @brief 样式输出流操作符
 * @param os 输出流
 * @param style 样式值
 * @return 输出流引用
 */
std::ostream& operator<<(std::ostream& os, Style style);

} // namespace webserver::color