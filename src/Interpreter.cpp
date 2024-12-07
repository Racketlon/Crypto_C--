#include "../include/Syntax.h"
#include "../include/Print.h"
#include "../include/Function.h"
#include "../include/Variables.h"
#include "../include/error.h"

#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

class Interpreter {
private:
    Syntax syntax;
    Print printModule;
    Function functionModule;
    Variables variables;

public:
    void interpret(const std::string& fileName) {
        std::ifstream file(fileName);
        if (!file.is_open()) {
            reportError(0, "", "Could not open file.");
            return;
        }

        std::string line;
        int lineNumber = 0;

        while (std::getline(file, line)) {
            ++lineNumber;
            line = trim(line);

            if (line.empty()) continue;

            try {
                interpretLine(line, lineNumber);
            } catch (const std::exception& e) {
                reportError(lineNumber, line, e.what());
            }
        }

        file.close();
    }

private:
    void interpretLine(const std::string& line, int lineNumber) {
        try {
            if (std::regex_match(line, syntax.getVariableRegex())) {
                std::smatch match;
                std::regex_match(line, match, syntax.getVariableRegex());
                std::string varName = match[1];
                std::string value = match[2];
                variables.setVariable(varName, value);
            } else if (std::regex_match(line, syntax.getPrintRegex())) {
                std::smatch match;
                std::regex_match(line, match, syntax.getPrintRegex());
                std::string content = match[1];
                processPrint(content, lineNumber);
            } else {
                throw std::runtime_error("Unknown command or syntax");
            }
        } catch (const std::exception& e) {
            reportError(lineNumber, line, e.what());
        }
    }

    void processPrint(const std::string& content, int lineNumber) {
        try {
            if (variables.hasVariable(content)) {
                VariableValue value = variables.getVariable(content);
                std::cout << variables.stringifyValue(value) << std::endl;
            } else if (std::regex_match(content, std::regex(R"(^(\w+)\[(\d+)\]$)"))) {
                std::smatch match;
                std::regex_match(content, match, std::regex(R"(^(\w+)\[(\d+)\]$)"));
                std::string varName = match[1];
                int index = std::stoi(match[2]);

                if (!variables.hasVariable(varName)) {
                    throw std::runtime_error("Undefined variable: " + varName);
                }

                VariableValue value = variables.getVariable(varName);
                if (!std::holds_alternative<std::vector<std::string>>(value)) {
                    throw std::runtime_error("Variable is not an array: " + varName);
                }

                const auto& array = std::get<std::vector<std::string>>(value);
                if (index < 0 || index >= static_cast<int>(array.size())) {
                    throw std::runtime_error("Index out of bounds: " + std::to_string(index));
                }

                std::cout << array[index] << std::endl;
            } else if (std::regex_match(content, std::regex(R"(^(\w+)\["(.+)\"]$)"))) {
                std::smatch match;
                std::regex_match(content, match, std::regex(R"(^(\w+)\["(.+)\"]$)"));
                std::string varName = match[1];
                std::string key = match[2];

                if (!variables.hasVariable(varName)) {
                    throw std::runtime_error("Undefined variable: " + varName);
                }

                VariableValue value = variables.getVariable(varName);
                if (!std::holds_alternative<std::map<std::string, int>>(value)) {
                    throw std::runtime_error("Variable is not a dictionary: " + varName);
                }

                const auto& dictionary = std::get<std::map<std::string, int>>(value);
                if (dictionary.find(key) == dictionary.end()) {
                    throw std::runtime_error("Key not found in dictionary: " + key);
                }

                std::cout << dictionary.at(key) << std::endl;
            } else {
                std::string evaluatedContent = replaceVariables(content);
                printModule.processPrint(evaluatedContent, syntax.getExpressionRegex(), functionModule, lineNumber);
            }
        } catch (const std::exception& e) {
            reportError(lineNumber, content, e.what());
        }
    }

    std::string replaceVariables(const std::string& content) {
        std::regex varRegex(R"(\{(\w+)\})");
        std::smatch match;
        std::string result = content;
        while (std::regex_search(result, match, varRegex)) {
            std::string varName = match[1];
            if (!variables.hasVariable(varName)) {
                throw std::runtime_error("Undefined variable: " + varName);
            }
            VariableValue value = variables.getVariable(varName);
            result.replace(match.position(), match.length(), variables.stringifyValue(value));
        }
        return result;
    }

    void reportError(int lineNumber, const std::string& line, const std::string& message) {
        const std::string red = "\033[1;31m";    // Bold Red
        const std::string reset = "\033[0m";     // Reset
        const std::string cyan = "\033[1;36m";   // Bold Cyan

        std::cerr << red << "❌ Error on line " << lineNumber << ": " << message << reset << "\n"
                  << cyan << "    " << line << reset << "\n";
    }

    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t");
        size_t last = str.find_last_not_of(" \t");
        return (first == std::string::npos || last == std::string::npos) ? "" : str.substr(first, last - first + 1);
    }
};
