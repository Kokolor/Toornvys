#pragma once

#include "node.hpp"

class NodeNumber : public Node
{
public:
	explicit NodeNumber(int value, int line)
		: value(value), line(line) {}

	int getValue() const { return value; }
	int getLine() const override { return line; }

private:
	int value;
	int line;
};