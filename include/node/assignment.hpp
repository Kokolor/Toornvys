#pragma once

#include "node.hpp"

class NodeAssignment : public Node
{
public:
    NodeAssignment(const std::string &name, std::unique_ptr<Node> value, int line)
        : name(name), value(std::move(value)), line(line) {}

    const std::string &getName() const { return name; }
    const Node *getValue() const { return value.get(); }
    int getLine() const override { return line; }

private:
    const std::string name;
    std::unique_ptr<Node> value;
    int line;
};

class NodePointerAssignment : public Node
{
public:
    NodePointerAssignment(std::unique_ptr<Node> ptr, std::unique_ptr<Node> value, int line)
        : ptr(std::move(ptr)), value(std::move(value)), line(line) {}

    const Node *getPointer() const { return ptr.get(); }
    const Node *getValue() const { return value.get(); }
    int getLine() const override { return line; }

private:
    std::unique_ptr<Node> ptr;
    std::unique_ptr<Node> value;
    int line;
};