#pragma once

#include "node.hpp"

class NodeUnaryOp : public Node
{
public:
	NodeUnaryOp(Token::Kind op, std::unique_ptr<Node> operand, int line) : op(op), operand(std::move(operand)), line(line) {}

	Token::Kind getOp() const { return op; }
	const Node *getOperand() const { return operand.get(); }
	int getLine() const override { return line; }

	std::unique_ptr<Node> operand;

private:
	Token::Kind op;
	int line;
};