#pragma once

#include "node.hpp"

class NodeFunctionDeclaration : public Node
{
public:
	NodeFunctionDeclaration(const std::string &name, const std::vector<std::pair<std::string, std::string>> &args, std::unique_ptr<Node> body, const std::string &returnType, int line)
		: name(name), args(args), body(std::move(body)), returnType(returnType), line(line) {}

	const std::string &getName() const { return name; }
	const std::vector<std::pair<std::string, std::string>> &getArgs() const { return args; }
	const std::unique_ptr<Node> &getBody() const { return body; }
	const std::string &getReturnType() const { return returnType; }
	int getLine() const override { return line; }

private:
	std::string name;
	std::vector<std::pair<std::string, std::string>> args;
	std::unique_ptr<Node> body;
	std::string returnType;
	int line;
};