#include "../include/Syntax.h"
#include "../include/Print.h"
#include "../include/Function.h"
#include "../include/error.h"

#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

class Interpreter {
private:
    Syntax syntax;
    Print printModule;
    Function functionModule;

public:
    void interpret(const std::string& fileName) {
        std::ifstream file(fileName);
        if (!file.is_open()) {
            Error::reportError(0, "", "Could not open file " + fileName);
            return;
        }

        std::string line;
        int lineNumber = 0;
        bool inFunctionDefinition = false;
        std::string functionName, functionParams;
        std::vector<std::string> functionBody;

        while (std::getline(file, line)) {
            ++lineNumber;

            line = trim(line);

            if (line.empty()) continue;

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

            try {
                if (std::regex_match(line, syntax.getFunctionDefinitionRegex())) {
                    std::smatch match;
                    std::regex_match(line, match, syntax.getFunctionDefinitionRegex());
                    functionName = match[1];
                    functionParams = match[2];
                    inFunctionDefinition = true;
                } else if (std::regex_match(line, syntax.getFunctionCallRegex())) {
                    std::smatch match;
                    std::regex_match(line, match, syntax.getFunctionCallRegex());
                    std::string functionName = match[1];
                    std::vector<std::string> args = parseArguments(match[2]);
                    functionModule.executeFunction(functionName, args, [&](const std::string& line) {
                        interpretLine(line, lineNumber);
                    });
                } else {
                    interpretLine(line, lineNumber);
                }
            } catch (const std::exception& e) {
                handleSyntaxErrors(line, lineNumber, e.what());
            }
        }

        file.close();
    }

private:
    void interpretLine(const std::string& line, int lineNumber) {
        try {
            if (std::regex_match(line, syntax.getLambdaRegex())) {
                // Define a lambda
                std::smatch match;
                std::regex_match(line, match, syntax.getLambdaRegex());
                functionModule.defineLambda(match[1], match[2], match[3]);
            } else if (std::regex_match(line, syntax.getPrintRegex())) {
                // Process a print statement
                std::smatch match;
                std::regex_match(line, match, syntax.getPrintRegex());
                printModule.processPrint(match[1], syntax.getExpressionRegex(), functionModule, lineNumber);
            } else if (line.substr(0, 5) == "print") {
                // Extract and process the print statement
                size_t openParen = line.find('(');
                size_t closeParen = line.rfind(')');
                if (openParen == std::string::npos || closeParen == std::string::npos || closeParen <= openParen) {
                    throw std::runtime_error("Invalid print syntax");
                }
                std::string expression = line.substr(openParen + 1, closeParen - openParen - 1);
                // Use processPrint to handle the expression
                printModule.processPrint(expression, syntax.getExpressionRegex(), functionModule, lineNumber);
            } else {
                handleSyntaxErrors(line, lineNumber);
            }
        } catch (const std::exception& e) {
            handleSyntaxErrors(line, lineNumber, e.what());
        }
    }

    std::vector<std::string> parseArguments(const std::string& args) {
        std::vector<std::string> parsedArgs;
        std::istringstream argStream(args);
        std::string arg;

        while (std::getline(argStream, arg, ',')) {
            arg = trim(arg); // Trim whitespace from each argument
            if (!arg.empty()) {
                parsedArgs.push_back(arg);
            }
        }

        return parsedArgs;
    }

    void handleSyntaxErrors(const std::string& line, int lineNumber, const std::string& additionalMessage = "") {
        // Check for missing parentheses
        if (line.find('(') != std::string::npos && line.find(')') == std::string::npos) {
            Error::reportDetailedError(lineNumber, line, "Missing closing ')'",
                                       "Ensure all opening parentheses have a matching closing parenthesis.");
            return;
        }

        // General unknown command or syntax error
        std::string message = additionalMessage.empty() ? "Unknown command or syntax" : additionalMessage;
        Error::reportDetailedError(lineNumber, line, message, "Check the line for typos or unsupported commands.");
    }

    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t");
        size_t last = str.find_last_not_of(" \t");
        return (first == std::string::npos || last == std::string::npos) ? "" : str.substr(first, last - first + 1);
    }
};
