#pragma once

#include "node.hpp"

class NodeVarDeclaration : public Node
{
public:
	NodeVarDeclaration(const std::string &name, std::string &type, std::unique_ptr<Node> initializer, int line) : name(name), type(type), initializer(std::move(initializer)), line(line) {}

	const std::string &getName() const { return name; }
	const std::string &getType() const { return type; }
	const Node *getInitializer() const { return initializer.get(); }
	int getLine() const override { return line; }

private:
	std::string name;
	std::string type;
	std::unique_ptr<Node> initializer;
	int line;
};