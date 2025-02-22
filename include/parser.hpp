#pragma once

#include <memory>
#include "lexer.hpp"

class Node {
public:
	virtual ~Node() = default;
};

class NodeNumber : public Node {
public:
	explicit NodeNumber(int value) : value(value) {}

	int getValue() const { return value; }

private:
	int value;
};

class NodeIdentifier : public Node {
public:
	explicit NodeIdentifier(const std::string &name) : name(name) {}

	const std::string &getName() const { return name; }

private:
	std::string name;
};

class NodeBinaryOp : public Node {
public:
	NodeBinaryOp(Token::Kind op, std::unique_ptr<Node> left, std::unique_ptr<Node> right) : op(op), left(std::move(left)), right(std::move(right)) {}

	Token::Kind getOp() const { return op; }
	const Node *getLeft() const { return left.get(); }
	const Node *getRight() const { return right.get(); }

private:
	Token::Kind op;
	std::unique_ptr<Node> left;
	std::unique_ptr<Node> right;
};

class Parser {
public:
	explicit Parser(std::vector<Token> &tokens) : tokens(tokens), position(0) {}

	std::unique_ptr<Node> parse();

private:
	std::unique_ptr<Node> parseExpression();
	std::unique_ptr<Node> parseTerm();
	std::unique_ptr<Node> parseFactor();

	bool matchMultipleTokens(const std::vector<Token::Kind> &kinds);
	bool matchSingleToken(const Token::Kind kind);
	bool isAtEnd() const;

	Token peek() const;
	Token previous() const;
	Token advance();

	const std::vector<Token> tokens;
	size_t position;
};
