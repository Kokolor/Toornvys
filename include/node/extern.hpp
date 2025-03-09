#pragma once

#include "node.hpp"

class NodeExternDeclaration : public Node
{
public:
    NodeExternDeclaration(const std::string name, const std::vector<std::pair<std::string, std::string>> &args, const std::string &returnType, int line)
        : name(std::move(name)), args(std::move(args)), returnType(returnType), line(line) {}

    const std::string &getName() const { return name; }
    const std::vector<std::pair<std::string, std::string>> &getArgs() const { return args; }
    const std::string &getReturnType() const { return returnType; }
    int getLine() const override { return line; }

private:
    std::string name;
    std::vector<std::pair<std::string, std::string>> args;
    std::string returnType;
    int line;
};