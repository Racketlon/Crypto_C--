#ifndef VARIABLES_H
#define VARIABLES_H

#include <string>
#include <variant>
#include <map>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdexcept>

// A variant type to support multiple data types
using VariableValue = std::variant<std::string, int, double, bool, std::vector<std::string>, std::map<std::string, int>>;

class Variables {
private:
    std::map<std::string, VariableValue> variables;

public:
    // Set a variable with automatic type detection
    void setVariable(const std::string& name, const std::string& value) {
        if (isInteger(value)) {
            variables[name] = std::stoi(value);
        } else if (isDouble(value)) {
            variables[name] = std::stod(value);
        } else if (isBoolean(value)) {
            variables[name] = (value == "true");
        } else if (isArray(value)) {
            variables[name] = parseArray(value);
        } else if (isDictionary(value)) {
            variables[name] = parseDictionary(value);
        } else {
            variables[name] = trimQuotes(value);
        }
    }

    // Get a variable
    VariableValue getVariable(const std::string& name) const {
        if (variables.find(name) == variables.end()) {
            throw std::runtime_error("Undefined variable: " + name);
        }
        return variables.at(name);
    }

    // Check if a variable exists
    bool hasVariable(const std::string& name) const {
        return variables.find(name) != variables.end();
    }

    // Add this method to stringify VariableValue
    std::string stringifyValue(const VariableValue& value) const {
        if (std::holds_alternative<std::string>(value)) {
            return std::get<std::string>(value);
        } else if (std::holds_alternative<int>(value)) {
            return std::to_string(std::get<int>(value));
        } else if (std::holds_alternative<double>(value)) {
            std::ostringstream oss;
            oss.precision(6);
            oss << std::fixed << std::get<double>(value);
            return oss.str();
        } else if (std::holds_alternative<bool>(value)) {
            return std::get<bool>(value) ? "true" : "false";
        } else if (std::holds_alternative<std::vector<std::string>>(value)) {
            std::ostringstream oss;
            oss << "[";
            const auto& vec = std::get<std::vector<std::string>>(value);
            for (size_t i = 0; i < vec.size(); ++i) {
                oss << "\"" << vec[i] << "\"";
                if (i != vec.size() - 1) oss << ", ";
            }
            oss << "]";
            return oss.str();
        } else if (std::holds_alternative<std::map<std::string, int>>(value)) {
            std::ostringstream oss;
            oss << "{";
            const auto& map = std::get<std::map<std::string, int>>(value);
            size_t count = 0;
            for (const auto& [key, val] : map) {
                oss << "\"" << key << "\": " << val;
                if (count != map.size() - 1) oss << ", ";
                ++count;
            }
            oss << "}";
            return oss.str();
        }
        return "";
    }

private:
    // Helper to check if a string is an integer
    bool isInteger(const std::string& value) const {
        return !value.empty() && value.find_first_not_of("-0123456789") == std::string::npos;
    }

    // Helper to check if a string is a double
    bool isDouble(const std::string& value) const {
        std::istringstream iss(value);
        double d;
        char c;
        return iss >> d && !(iss >> c);
    }

    // Helper to check if a string is a boolean
    bool isBoolean(const std::string& value) const {
        return value == "true" || value == "false";
    }

    // Helper to check if a string is an array
    bool isArray(const std::string& value) const {
        return value.front() == '[' && value.back() == ']';
    }

    // Helper to check if a string is a dictionary
    bool isDictionary(const std::string& value) const {
        return value.front() == '{' && value.back() == '}';
    }

    // Helper to parse an array from a string
    std::vector<std::string> parseArray(const std::string& value) const {
        std::vector<std::string> result;
        std::string content = value.substr(1, value.size() - 2); // Remove brackets
        std::istringstream stream(content);
        std::string item;

        while (std::getline(stream, item, ',')) {
            result.push_back(trimQuotes(trim(item)));
        }

        return result;
    }

    // Helper to parse a dictionary from a string
    std::map<std::string, int> parseDictionary(const std::string& value) const {
        std::map<std::string, int> result;
        std::string content = value.substr(1, value.size() - 2); // Remove braces
        std::istringstream stream(content);
        std::string pair;

        while (std::getline(stream, pair, ',')) {
            size_t colonPos = pair.find(':');
            if (colonPos == std::string::npos) {
                throw std::runtime_error("Invalid dictionary format: " + value);
            }
            std::string key = trimQuotes(trim(pair.substr(0, colonPos)));
            int val = std::stoi(trim(pair.substr(colonPos + 1)));
            result[key] = val;
        }

        return result;
    }

    // Helper to trim whitespace
    std::string trim(const std::string& str) const {
        size_t first = str.find_first_not_of(" \t");
        size_t last = str.find_last_not_of(" \t");
        return (first == std::string::npos || last == std::string::npos)
                   ? ""
                   : str.substr(first, last - first + 1);
    }

    // Helper to trim quotes
    std::string trimQuotes(const std::string& str) const {
        if (str.size() >= 2 && ((str.front() == '"' && str.back() == '"') ||
                                (str.front() == '\'' && str.back() == '\''))) {
            return str.substr(1, str.size() - 2);
        }
        return str;
    }
};

#endif
