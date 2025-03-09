#pragma once

#include "node.hpp"

class NodeReturn : public Node
{
public:
    explicit NodeReturn(std::unique_ptr<Node> expression, int line)
        : expression(std::move(expression)), line(line) {}

    const Node *getExpression() const { return expression.get(); }
    int getLine() const override { return line; }

private:
    std::unique_ptr<Node> expression;
    int line;
};