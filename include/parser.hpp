#pragma once

#include <vector>
#include <memory>
#include "lexer.hpp"
#include "node/node.hpp"
#include "node/block.hpp"
#include "node/op.hpp"
#include "node/identifier.hpp"
#include "node/number.hpp"
#include "node/string.hpp"
#include "node/functionCall.hpp"
#include "node/variableDeclaration.hpp"
#include "node/functionDeclaration.hpp"
#include "node/return.hpp"
#include "node/array.hpp"
#include "node/extern.hpp"
#include "node/cast.hpp"
#include "node/assignment.hpp"

class Parser
{
	std::vector<Token> tokens;
	size_t position = 0;

public:
	explicit Parser(std::vector<Token> &tokens)
		: tokens(tokens) {}

	std::unique_ptr<Node> parse();

private:
	std::unique_ptr<Node> parseExpression();
	std::unique_ptr<Node> parseComparison();
	std::unique_ptr<Node> parseAdditive();
	std::unique_ptr<Node> parseTerm();
	std::unique_ptr<Node> parseFactor();
	std::unique_ptr<Node> parsePrimary();
	std::unique_ptr<Node> parseUnary();

	std::unique_ptr<Node> parseNumberLiteral();
	std::unique_ptr<Node> parseStringLiteral();
	std::unique_ptr<Node> parseIdentifierExpression();
	std::unique_ptr<Node> parseParenthesizedExpression();
	std::unique_ptr<Node> parseCastOperation(std::unique_ptr<Node> expr);
	std::unique_ptr<Node> parseFunctionCall(const std::string &name, int line);
	std::unique_ptr<Node> parseArrayAccess(const std::string &name, int line);

	std::string parseType();

	std::vector<std::pair<std::string, std::string>> parseArgumentList();
	std::pair<std::string, std::string> parseArgument();

	std::unique_ptr<Node> parseStatement();
	std::unique_ptr<NodeBlock> parseBlock();
	std::unique_ptr<Node> parseReturn();
	std::unique_ptr<Node> parseVariableDeclaration();
	std::unique_ptr<Node> parseFunctionDeclaration();
	std::unique_ptr<Node> parseExternDeclaration();
	std::unique_ptr<Node> parseAssignment();

	bool matchMultipleTokens(const std::vector<Token::Kind> &kinds);
	bool matchSingleToken(Token::Kind kind);
	Token consumeToken(Token::Kind expected, const std::string &errorMsg);
	Token consumeToken();
	Token advance();
	bool isAtEnd() const;
	Token peek() const;
	Token previous() const;
};