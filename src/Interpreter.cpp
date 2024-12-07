#include "../include/Syntax.h"
#include "../include/Print.h"
#include "../include/Function.h"
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

public:
    void interpret(const std::string& fileName) {
        std::ifstream file(fileName);
        if (!file.is_open()) {
            reportError(0, "", "Could not open file.");
            return;
        }

        std::string line;
        int lineNumber = 0;
        bool inFunctionDefinition = false;
        bool inMultiLineComment = false;
        std::string functionName, functionParams;
        std::vector<std::string> functionBody;

        while (std::getline(file, line)) {
            ++lineNumber;

            // Trim line
            line = trim(line);

            // Handle multi-line comments
            if (inMultiLineComment) {
                if (line.find("*/") != std::string::npos) {
                    inMultiLineComment = false;
                }
                continue;
            }
            if (line.find("/*") != std::string::npos) {
                inMultiLineComment = true;
                continue;
            }

            // Ignore single-line comments
            size_t commentPos = line.find("//");
            if (commentPos != std::string::npos) {
                line = line.substr(0, commentPos);
                line = trim(line);
            }

            // Skip empty lines
            if (line.empty()) continue;

            // Handle function definitions and function calls
            try {
                if (inFunctionDefinition) {
                    if (line == "}") {
                        functionModule.defineFunction(functionName, functionParams, functionBody);
                        inFunctionDefinition = false;
                        functionBody.clear();
                        continue;
                    }
                    functionBody.push_back(line);
                    continue;
                }

                if (std::regex_match(line, syntax.getFunctionDefinitionRegex())) {
                    std::smatch match;
                    std::regex_match(line, match, syntax.getFunctionDefinitionRegex());
                    functionName = match[1];
                    functionParams = match[2];
                    inFunctionDefinition = true;
                } else if (std::regex_match(line, syntax.getPrintRegex())) {
                    // Explicitly handle `print` statements
                    std::smatch match;
                    std::regex_match(line, match, syntax.getPrintRegex());
                    printModule.processPrint(match[1], syntax.getExpressionRegex(), functionModule, lineNumber);
                } else if (std::regex_match(line, syntax.getFunctionCallRegex())) {
                    std::smatch match;
                    std::regex_match(line, match, syntax.getFunctionCallRegex());
                    std::string functionName = match[1];
                    std::vector<std::string> args = parseArguments(match[2]);
                    functionModule.executeFunction(functionName, args, [&](const std::string& line) {
                        interpretLine(line, lineNumber);
                    });
                } else {
                    throw std::runtime_error("Unknown command or syntax");
                }
            } catch (const std::exception& e) {
                reportError(lineNumber, line, e.what());
            }
        }

        file.close();
    }

private:
    void interpretLine(const std::string& line, int lineNumber) {
        try {
            if (std::regex_match(line, syntax.getLambdaRegex())) {
                std::smatch match;
                std::regex_match(line, match, syntax.getLambdaRegex());
                functionModule.defineLambda(match[1], match[2], match[3]);
            } else if (std::regex_match(line, syntax.getPrintRegex())) {
                // Explicitly handle `print` statements
                std::smatch match;
                std::regex_match(line, match, syntax.getPrintRegex());
                printModule.processPrint(match[1], syntax.getExpressionRegex(), functionModule, lineNumber);
            } else if (std::regex_match(line, syntax.getFunctionCallRegex())) {
                std::smatch match;
                std::regex_match(line, match, syntax.getFunctionCallRegex());
                std::string functionName = match[1];
                std::vector<std::string> args = parseArguments(match[2]);
                functionModule.executeFunction(functionName, args, [&](const std::string& line) {
                    interpretLine(line, lineNumber);
                });
            } else {
                throw std::runtime_error("Unknown command or syntax");
            }
        } catch (const std::exception& e) {
            reportError(lineNumber, line, e.what());
        }
    }

    std::vector<std::string> parseArguments(const std::string& args) {
        std::vector<std::string> parsedArgs;
        std::istringstream argStream(args);
        std::string arg;

        while (std::getline(argStream, arg, ',')) {
            arg = trim(arg);
            if (!arg.empty()) {
                parsedArgs.push_back(arg);
            }
        }

        return parsedArgs;
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
