#ifndef PRINT_H
#define PRINT_H

#include <iostream>
#include <string>
#include <regex>
#include <functional>
#include <cmath>
#include <cstring>
#include "../include/Function.h"
#include "../include/error.h"

class Print {
public:
    void processPrint(const std::string& content, const std::regex& expressionRegex,
                      Function& functionModule, int lineNumber) {
        std::smatch match;
        std::string result;
        std::string remaining = content;

        // Process embedded expressions in {}
        while (std::regex_search(remaining, match, expressionRegex)) {
            result += remaining.substr(0, match.position());
            std::string expression = match[1].str();

            try {
                std::string evaluated = evaluateExpression(expression, functionModule);
                result += evaluated;
            } catch (const std::exception& e) {
                Error::reportDetailedError(lineNumber, content, e.what(),
                                           "Ensure the expression inside '{}' is valid.");
                return;
            }

            remaining = match.suffix();
        }

        // Append remaining text or treat it as plain text
        result += remaining;

        std::cout << result << std::endl;
    }

private:
    std::string evaluateExpression(const std::string& expression, Function& functionModule) {
        if (isStringLiteral(expression)) {
            // Return string literal without quotes
            return expression.substr(1, expression.length() - 2);
        }

        try {
            double result = evaluateMathExpression(expression, functionModule);
            return formatNumber(result);
        } catch (...) {
            throw std::runtime_error("Invalid expression: " + expression);
        }
    }

    double evaluateMathExpression(const std::string& expression, Function& functionModule) {
        std::regex lambdaCallRegex(R"((\w+)\((.*?)\))");
        std::smatch match;

        if (std::regex_match(expression, match, lambdaCallRegex)) {
            std::string functionName = match[1];
            std::string args = match[2];

            std::vector<double> argValues;
            std::istringstream argStream(args);
            std::string arg;
            while (std::getline(argStream, arg, ',')) {
                arg = trim(arg);
                if (arg.empty()) {
                    throw std::runtime_error("Empty argument in function or lambda call.");
                }
                argValues.push_back(evaluateMathExpression(arg, functionModule));
            }

            return functionModule.evaluateLambdaOrFunction(functionName, argValues, [&](const std::string& logic) {
                return evaluateMathExpression(logic, functionModule);
            });
        }

        try {
            return std::stod(expression); // Attempt to convert to a number
        } catch (...) {
            throw std::runtime_error("Invalid numeric value: " + expression);
        }
    }

    bool isStringLiteral(const std::string& expression) const {
        return expression.size() >= 2 && expression.front() == '"' && expression.back() == '"';
    }

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

    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t");
        size_t last = str.find_last_not_of(" \t");
        return (first == std::string::npos || last == std::string::npos) ? "" : str.substr(first, last - first + 1);
    }
};

#endif
