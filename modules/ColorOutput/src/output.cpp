#include <webserver/color/output.hpp>
#include <iostream>

namespace webserver::color {

namespace {
    // 检查是否应该输出颜色
    bool should_output_color() {
        static const bool no_color = [](){
            const char* env = std::getenv("NO_COLOR");
            return env != nullptr && env[0] != '\0';
        }();
        return !no_color && (&std::cout == &std::cerr ? std::cerr.rdbuf() : std::cout.rdbuf());
    }
} // namespace

std::ostream& operator<<(std::ostream& os, Color color) {
    if (should_output_color()) {
        os << "\033[" << static_cast<int>(color) << "m";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, Background bg) {
    if (should_output_color()) {
        os << "\033[" << static_cast<int>(bg) << "m";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, Style style) {
    if (should_output_color()) {
        os << "\033[" << static_cast<int>(style) << "m";
    }
    return os;
}

} // namespace webserver::color