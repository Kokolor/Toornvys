#pragma once

#include "node.hpp"

class NodeBinaryOp : public Node
{
public:
	NodeBinaryOp(Token::Kind op, std::unique_ptr<Node> left, std::unique_ptr<Node> right, int line) : op(op), left(std::move(left)), right(std::move(right)), line(line) {}

	Token::Kind getOp() const { return op; }
	const Node *getLeft() const { return left.get(); }
	const Node *getRight() const { return right.get(); }
	int getLine() const override { return line; }

private:
	Token::Kind op;
	std::unique_ptr<Node> left;
	std::unique_ptr<Node> right;
	int line;
};