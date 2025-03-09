#pragma once

#include "node.hpp"

class NodeWhile : public Node
{
public:
    NodeWhile(std::unique_ptr<Node> condition, std::unique_ptr<Node> body, int line)
        : condition(std::move(condition)), body(std::move(body)), line(line) {}

    const Node *getCondition() const { return condition.get(); }
    const Node *getBody() const { return body.get(); }
    int getLine() const override { return line; }

private:
    std::unique_ptr<Node> condition;
    std::unique_ptr<Node> body;
    int line;
};