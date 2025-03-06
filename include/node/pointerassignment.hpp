#pragma once

#include "node.hpp"

class NodePointerAssignment : public Node
{
public:
    NodePointerAssignment(std::unique_ptr<Node> ptr, std::unique_ptr<Node> value, int line) : ptr(std::move(ptr)), value(std::move(value)), line(line) {}

    const Node *getPointer() const { return ptr.get(); }
    const Node *getValue() const { return value.get(); }
    int getLine() const override { return line; }

private:
    std::unique_ptr<Node> ptr;
    std::unique_ptr<Node> value;
    int line;
};