#pragma once

#include "node.hpp"

class NodeFunctionCall : public Node
{
public:
	NodeFunctionCall(const std::string &name, std::vector<std::unique_ptr<Node>> args, int line)
		: name(name), args(std::move(args)), line(line) {}

	const std::string &getName() const { return name; }
	const std::vector<std::unique_ptr<Node>> &getArgs() const { return args; }
	int getLine() const override { return line; }

private:
	std::string name;
	std::vector<std::unique_ptr<Node>> args;
	int line;
};