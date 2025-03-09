#pragma once

#include "node.hpp"

class NodeString : public Node
{
public:
    NodeString(const std::string &value, int line)
        : value(value), line(line) {}

    const std::string &getValue() const { return value; }
    int getLine() const override { return line; }

private:
    std::string value;
    int line;
};