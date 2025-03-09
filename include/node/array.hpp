#pragma once

#include "node.hpp"
#include <memory>

class NodeArrayAccess : public Node
{
public:
    NodeArrayAccess(std::string name, std::unique_ptr<Node> index, int line)
        : name(std::move(name)), index(std::move(index)), line(line) {}

    const std::string &getName() const { return name; }
    const Node *getIndex() const { return index.get(); }
    std::unique_ptr<Node> releaseIndex() { return std::move(index); }
    int getLine() const override { return line; }
    std::string name;
    std::unique_ptr<Node> index;
    int line;
};

class NodeArrayAssignment : public Node
{
public:
    NodeArrayAssignment(std::string name, std::unique_ptr<Node> index, std::unique_ptr<Node> value, int line)
        : name(std::move(name)), index(std::move(index)), value(std::move(value)), line(line) {}

    const std::string &getName() const { return name; }
    const Node *getIndex() const { return index.get(); }
    const Node *getValue() const { return value.get(); }
    int getLine() const override { return line; }

private:
    std::string name;
    std::unique_ptr<Node> index, value;
    int line;
};

class NodeArrayLiteral : public Node
{
public:
    NodeArrayLiteral(std::vector<std::unique_ptr<Node>> elements, int line)
        : elements(std::move(elements)), line(line) {}

    const auto &getElements() const { return elements; }
    int getLine() const override { return line; }

private:
    std::vector<std::unique_ptr<Node>> elements;
    int line;
};