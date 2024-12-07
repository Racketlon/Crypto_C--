#ifndef ERROR_H
#define ERROR_H

#include <iostream>
#include <string>

class Error {
public:
    static void reportError(int line, const std::string& code, const std::string& message) {
        // Error header with ❌
        std::cerr << "\033[1;31m❌ Error\033[0m on line \033[1;33m" << line << "\033[0m:" << std::endl;

        // Display the line causing the error
        std::cerr << "    " << code << std::endl;

        // Highlight the error position if possible
        size_t errorStart = code.find_first_not_of(" \t");
        if (errorStart != std::string::npos) {
            size_t length = code.length() - errorStart;
            std::cerr << "    \033[1;31m" << std::string(errorStart, ' ') << std::string(length, '^') << "\033[0m" << std::endl;
        }

        // Detailed error message
        std::cerr << "    \033[1;31m" << message << "\033[0m" << std::endl;

        // Add spacing for readability
        std::cerr << std::endl;
    }

    static void reportDetailedError(int line, const std::string& code, const std::string& message,
                                    const std::string& suggestion) {
        reportError(line, code, message);
        std::cerr << "\033[1;36mSuggestion:\033[0m " << suggestion << std::endl << std::endl;
    }
};

#endif
