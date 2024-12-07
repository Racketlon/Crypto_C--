#ifndef SYNTAX_H
#define SYNTAX_H

#include <string>
#include <regex>

class Syntax {
public:
    std::string getPrintKeyword() const { return "print"; }
    std::regex getLambdaRegex() const { return std::regex(R"((\w+)\((.*?)\)\s*=>\s*(.*))"); }
    std::regex getPrintRegex() const { return std::regex(R"(^print\s*\((.*)\)\s*$)"); }
    std::regex getExpressionRegex() const { return std::regex(R"(\{(.*?)\})"); }
    std::regex getCommentRegex() const { return std::regex(R"(\s*//.*)"); }
    std::regex getMultiLineCommentStartRegex() const { return std::regex(R"(\s*/\*.*)"); }
    std::regex getMultiLineCommentEndRegex() const { return std::regex(R"(.*\*/\s*)"); }
    std::regex getFunctionDefinitionRegex() const { return std::regex(R"(fn\s+(\w+)\((.*?)\)\s*\{)"); }
    std::regex getFunctionCallRegex() const { return std::regex(R"((\w+)\((.*?)\))"); }
    std::regex getVariableRegex() const { return std::regex(R"(^(\w+)\s*=\s*(.*)$)"); }
};

#endif
