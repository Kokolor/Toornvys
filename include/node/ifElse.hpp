#pragma once

#include "node.hpp"

class NodeIf : public Node
{
public:
    NodeIf(std::unique_ptr<Node> condition, std::unique_ptr<Node> thenBranch, std::unique_ptr<Node> elseBranch, int line)
        : condition(std::move(condition)), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)), line(line) {}

    const Node *getCondition() const { return condition.get(); }
    const Node *getThenBranch() const { return thenBranch.get(); }
    const Node *getElseBranch() const { return elseBranch.get(); }
    int getLine() const override { return line; }

private:
    std::unique_ptr<Node> condition;
    std::unique_ptr<Node> thenBranch;
    std::unique_ptr<Node> elseBranch;
    int line;
};