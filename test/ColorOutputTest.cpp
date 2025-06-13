#include <iostream>
#include "../include/ColorOutput.hpp"

int main() {
    using namespace ColorOutput;

    info("This is an information message");
    warn("This is a warning message");
    error("This is an error message");
    debug("This is a debug message");

    println("Custom color message", ColorOutput::Color::BRIGHT_CYAN);
    
    print("First part ", ColorOutput::Color::GREEN);
    print("Second part ", ColorOutput::Color::YELLOW);
    println("Third part", ColorOutput::Color::RED);

    return 0;
}