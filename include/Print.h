#ifndef PRINT_H
#define PRINT_H

#include <iostream>
#include <string>
#include <regex>
#include <functional>
#include <cmath>
#include <sstream>
#include "../include/Function.h"
#include "../include/error.h"

class Print {
public:
    // Process the print statement
    void processPrint(const std::string& content, const std::regex& expressionRegex,
                      Function& functionModule, int lineNumber) {
        std::smatch match;
        std::string result;
        std::string remaining = content;

        // Process embedded expressions in {}
        while (std::regex_search(remaining, match, expressionRegex)) {
            // Add text before the match
            result += remaining.substr(0, match.position());
            std::string expression = match[1].str();

            try {
                // Evaluate the expression
                std::string value = evaluateExpression(expression, functionModule);
                result += value;
            } catch (const std::exception& e) {
                reportError(lineNumber, content, e.what());
                return;
            }

            // Move to the remaining part of the string
            remaining = match.suffix();
        }

        // Append the rest of the string
        result += remaining;

        // Print the final result without quotes
        std::cout << trimQuotes(result) << std::endl;
    }

private:
    // Evaluate an expression, returning either a numeric or string value
    std::string evaluateExpression(const std::string& expression, Function& functionModule) {
        std::regex lambdaCallRegex(R"((\w+)\((.*?)\))");
        std::smatch match;

        // Check if the expression matches a lambda or function call
        if (std::regex_match(expression, match, lambdaCallRegex)) {
            std::string functionName = match[1];
            std::string args = match[2];

            // Parse arguments for the lambda or function
            std::vector<double> argValues = parseArguments(args, functionModule);

            // Evaluate the lambda or function dynamically
            double numericResult = functionModule.evaluateLambdaOrFunction(functionName, argValues, [&](const std::string& logic) {
                return std::stod(evaluateExpression(logic, functionModule));
            });

            return formatNumber(numericResult);
        }

        // Try converting to a number
        try {
            return formatNumber(std::stod(expression));
        } catch (...) {
            // If not numeric, treat as a literal string without quotes
            return trimQuotes(expression);
        }
    }

    // Parse arguments for lambdas or functions
    std::vector<double> parseArguments(const std::string& args, Function& functionModule) {
        std::vector<double> parsedArgs;
        std::istringstream argStream(args);
        std::string arg;

        while (std::getline(argStream, arg, ',')) {
            arg = trim(arg);
            if (!arg.empty()) {
                parsedArgs.push_back(std::stod(evaluateExpression(arg, functionModule)));
            }
        }

        return parsedArgs;
    }

    // Format a number for printing
    std::string formatNumber(double number) {
        if (std::fabs(number - std::round(number)) < 1e-9) {
            return std::to_string(static_cast<long long>(number));
        } else {
            std::ostringstream oss;
            oss.precision(6);
            oss << std::fixed << number;
            return oss.str();
        }
    }

    // Trim leading and trailing quotes from a string
    std::string trimQuotes(const std::string& str) {
        if (str.size() >= 2 && str.front() == '"' && str.back() == '"') {
            return str.substr(1, str.size() - 2);
        }
        return str;
    }

    // Trim whitespace from a string
    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t");
        size_t last = str.find_last_not_of(" \t");
        return (first == std::string::npos || last == std::string::npos) ? "" : str.substr(first, last - first + 1);
    }

    // Report errors
    void reportError(int lineNumber, const std::string& line, const std::string& message) {
        const std::string red = "\033[1;31m";    // Bold Red
        const std::string reset = "\033[0m";     // Reset
        const std::string cyan = "\033[1;36m";   // Bold Cyan

        std::cerr << red << "❌ Error on line " << lineNumber << ": " << message << reset << "\n"
                  << cyan << "    " << line << reset << "\n";
    }
};

#endif
