#pragma once

#include "node.hpp"

class NodeAssignment : public Node
{
public:
    NodeAssignment(const std::string &name, std::unique_ptr<Node> value, int line) : name(name), value(std::move(value)), line(line) {}

    const std::string &getName() const { return name; }
    const Node *getValue() const { return value.get(); }
    int getLine() const override { return line; }

private:
    const std::string name;
    std::unique_ptr<Node> value;
    int line;
};