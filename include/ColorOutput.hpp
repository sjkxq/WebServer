#pragma once
#include <iostream>
#include <string>

namespace ColorOutput {
    // ANSI color codes
    namespace Color {
        const std::string RESET = "\033[0m";
        const std::string BLACK = "\033[30m";
        const std::string RED = "\033[31m";
        const std::string GREEN = "\033[32m";
        const std::string YELLOW = "\033[33m";
        const std::string BLUE = "\033[34m";
        const std::string MAGENTA = "\033[35m";
        const std::string CYAN = "\033[36m";
        const std::string WHITE = "\033[37m";
        const std::string BRIGHT_RED = "\033[91m";
        const std::string BRIGHT_GREEN = "\033[92m";
        const std::string BRIGHT_YELLOW = "\033[93m";
        const std::string BRIGHT_BLUE = "\033[94m";
        const std::string BRIGHT_MAGENTA = "\033[95m";
        const std::string BRIGHT_CYAN = "\033[96m";
    }

    // Check if terminal supports color
    inline bool isColorSupported() {
        const char* term = std::getenv("TERM");
        if (term == nullptr) return false;
        return std::string(term) != "dumb";
    }

    // Colored output functions
    void print(const std::string& message, const std::string& color) {
        if (isColorSupported()) {
            std::cout << color << message << Color::RESET;
        } else {
            std::cout << message;
        }
    }

    void println(const std::string& message, const std::string& color) {
        print(message + "\n", color);
    }

    // Predefined message types
    void info(const std::string& message) {
        println("[INFO] " + message, Color::GREEN);
    }

    void warn(const std::string& message) {
        println("[WARN] " + message, Color::YELLOW);
    }

    void error(const std::string& message) {
        println("[ERROR] " + message, Color::RED);
    }

    void debug(const std::string& message) {
        println("[DEBUG] " + message, Color::BLUE);
    }
}