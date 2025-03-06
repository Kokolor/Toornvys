#pragma once

#include <memory>
#include "lexer.hpp"
#include "node/assignment.hpp"
#include "node/binaryop.hpp"
#include "node/block.hpp"
#include "node/funcdeclaration.hpp"
#include "node/functioncall.hpp"
#include "node/identifier.hpp"
#include "node/node.hpp"
#include "node/number.hpp"
#include "node/pointerassignment.hpp"
#include "node/return.hpp"
#include "node/unaryop.hpp"
#include "node/vardeclaration.hpp"

class Parser
{
public:
	explicit Parser(std::vector<Token> &tokens) : tokens(tokens), position(0) {}

	std::unique_ptr<Node> parse();

private:
	std::unique_ptr<Node> parseExpression();
	std::unique_ptr<Node> parseComparison();
	std::unique_ptr<Node> parseAdditive();
	std::unique_ptr<Node> parsePrimary();
	std::unique_ptr<Node> parseUnary();
	std::unique_ptr<Node> parseTerm();
	std::unique_ptr<Node> parseFactor();

	std::unique_ptr<Node> parseStatement();
	std::unique_ptr<NodeBlock> parseBlock();
	std::unique_ptr<Node> parseReturn();
	std::unique_ptr<Node> parseVariableDeclaration();
	std::unique_ptr<Node> parseFuncDeclaration();
	std::unique_ptr<Node> parseAssignment();

	bool matchMultipleTokens(const std::vector<Token::Kind> &kinds);
	bool matchSingleToken(const Token::Kind kind);
	bool isAtEnd() const;

	Token peek() const;
	Token previous() const;
	Token advance();

	const std::vector<Token> tokens;
	size_t position;
};
