#pragma once

#include "node.hpp"

class NodeBlock : public Node
{
public:
	explicit NodeBlock(int line)
		: line(line) {}

	void addStatement(std::unique_ptr<Node> stmt)
	{
		statements.push_back(std::move(stmt));
	}

	const std::vector<std::unique_ptr<Node>> &getStatements() const { return statements; }
	int getLine() const override { return line; }

private:
	std::vector<std::unique_ptr<Node>> statements;
	int line;
};