#pragma once

#include "node.hpp"

class NodeCast : public Node
{
public:
    NodeCast(const std::string &targetType, std::unique_ptr<Node> expression, int line)
        : targetType(targetType), expression(std::move(expression)), line(line) {}

    const std::string &getTargetType() const { return targetType; }
    const Node *getExpression() const { return expression.get(); }
    int getLine() const override { return line; }

private:
    std::string targetType;
    std::unique_ptr<Node> expression;
    int line;
};