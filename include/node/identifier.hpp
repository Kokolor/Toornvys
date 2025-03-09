#pragma once

#include "node.hpp"

class NodeIdentifier : public Node
{
public:
	explicit NodeIdentifier(const std::string &name, int line)
		: name(name), line(line) {}

	const std::string &getName() const { return name; }
	int getLine() const override { return line; }

private:
	std::string name;
	int line;
};