#include <iostream>
#include "webserver/color/output.hpp"

int main() {
    using namespace webserver::color;

    std::cout << "This is an information message" << std::endl;
    std::cout << Color::YELLOW << "This is a warning message" << Style::RESET << std::endl;
    std::cout << Color::RED << "This is an error message" << Style::RESET << std::endl;
    std::cout << Color::BLUE << "This is a debug message" << Style::RESET << std::endl;

    std::cout << Color::CYAN << "Custom color message" << Style::RESET << std::endl;
    
    std::cout << Color::GREEN << "First part " << Style::RESET;
    std::cout << Color::YELLOW << "Second part " << Style::RESET;
    std::cout << Color::RED << "Third part" << Style::RESET << std::endl;

    return 0;
}