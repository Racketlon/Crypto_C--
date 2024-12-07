#ifndef FUNCTION_H
#define FUNCTION_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <stdexcept>
#include <regex>
#include <sstream>

class Function {
private:
    // Store lambdas
    std::map<std::string, std::pair<std::vector<std::string>, std::string>> lambdas;

    // Store functions: name -> (parameters, body)
    std::map<std::string, std::pair<std::vector<std::string>, std::vector<std::string>>> functions;

public:
    // Define a new lambda
    void defineLambda(const std::string& name, const std::string& params, const std::string& logic) {
        std::vector<std::string> paramNames = parseParameters(params);
        lambdas[name] = {paramNames, logic};
    }

    // Define a new function
    void defineFunction(const std::string& name, const std::string& params, const std::vector<std::string>& body) {
        std::vector<std::string> paramNames = parseParameters(params);
        functions[name] = {paramNames, body};
    }

    // Execute a function dynamically
    void executeFunction(const std::string& name, const std::vector<std::string>& args,
                         const std::function<void(const std::string&)>& executeLine) {
        if (functions.find(name) == functions.end()) {
            throw std::runtime_error("Undefined function: " + name);
        }

        const auto& [paramNames, body] = functions[name];

        if (args.size() != paramNames.size()) {
            throw std::runtime_error("Function '" + name + "' expects " + std::to_string(paramNames.size()) +
                                     " arguments but got " + std::to_string(args.size()));
        }

        // Replace parameters with arguments in the function body
        for (const auto& line : body) {
            std::string replacedLine = line;
            for (size_t i = 0; i < paramNames.size(); ++i) {
                replacedLine = std::regex_replace(replacedLine, std::regex("\\b" + paramNames[i] + "\\b"), args[i]);
            }
            // Execute the replaced line
            executeLine(replacedLine);
        }
    }

    // Evaluate a lambda or function dynamically
    double evaluateLambdaOrFunction(const std::string& name, const std::vector<double>& args,
                                    const std::function<double(const std::string&)>& evaluateExpression) {
        if (lambdas.find(name) != lambdas.end()) {
            return evaluateLambda(name, args, evaluateExpression);
        }

        if (functions.find(name) != functions.end()) {
            throw std::runtime_error("Functions cannot return values directly.");
        }

        throw std::runtime_error("Undefined lambda or function: " + name);
    }

    // Evaluate a lambda dynamically
    double evaluateLambda(const std::string& name, const std::vector<double>& args,
                          const std::function<double(const std::string&)>& evaluateExpression) {
        if (lambdas.find(name) == lambdas.end()) {
            throw std::runtime_error("Undefined lambda: " + name);
        }

        const auto& [paramNames, logic] = lambdas[name];

        if (args.size() != paramNames.size()) {
            throw std::runtime_error("Lambda '" + name + "' expects " + std::to_string(paramNames.size()) +
                                     " arguments but got " + std::to_string(args.size()));
        }

        std::string evaluatedLogic = logic;
        for (size_t i = 0; i < paramNames.size(); ++i) {
            evaluatedLogic = std::regex_replace(evaluatedLogic, std::regex("\\b" + paramNames[i] + "\\b"), std::to_string(args[i]));
        }

        return evaluateExpression(evaluatedLogic);
    }

    // Get all lambdas
    const std::map<std::string, std::pair<std::vector<std::string>, std::string>>& getLambdas() const {
        return lambdas;
    }

private:
    std::vector<std::string> parseParameters(const std::string& params) {
        std::vector<std::string> paramNames;
        std::istringstream paramStream(params);
        std::string param;
        while (std::getline(paramStream, param, ',')) {
            param = trim(param);
            if (!param.empty()) {
                paramNames.push_back(param);
            }
        }
        return paramNames;
    }

    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t");
        size_t last = str.find_last_not_of(" \t");
        return (first == std::string::npos || last == std::string::npos) ? "" : str.substr(first, last - first + 1);
    }
};

#endif
