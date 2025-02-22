#include <stdexcept>
#include "../include/parser.hpp"

std::unique_ptr<Node> Parser::parse() {
	return parseExpression();
}

std::unique_ptr<Node> Parser::parseExpression() {
	auto left = parseTerm();

	while(matchMultipleTokens({Token::Kind::TOKEN_PLUS, Token::Kind::TOKEN_MINUS})) {
		Token::Kind op = previous().getKind();
		auto right = parseTerm();
		left = std::make_unique<NodeBinaryOp>(op, std::move(left), std::move(right));
	}

	return left;
}

std::unique_ptr<Node> Parser::parseTerm() {
	auto left = parseFactor();

	while(matchMultipleTokens({Token::Kind::TOKEN_PLUS, Token::Kind::TOKEN_MINUS})) {
		Token::Kind op = previous().getKind();
		auto right = parseFactor();
		left = std::make_unique<NodeBinaryOp>(op, std::move(left), std::move(right));
	}

	return left;
}

std::unique_ptr<Node> Parser::parseFactor() {
	if (matchSingleToken(Token::Kind::TOKEN_NUMBER)) {
		return std::make_unique<NodeNumber>(std::stoi(previous().getValue()));
	}

	if (matchSingleToken(Token::Kind::TOKEN_IDENTIFIER)) {
		return std::make_unique<NodeIdentifier>(previous().getValue());
	}

	throw std::runtime_error("Unexpected token in factor");
}

bool Parser::matchMultipleTokens(const std::vector<Token::Kind> &kinds) {
	for (auto kind : kinds) {
		if (matchSingleToken(kind)) {
			advance();

			return true;
		}
	}

	return false;
}

bool Parser::matchSingleToken(const Token::Kind kind) {
	if (isAtEnd())
		position++;

	return peek().getKind() == kind;
}

bool Parser::isAtEnd() const {
	return peek().getKind() == Token::Kind::TOKEN_EOF;
}

Token Parser::peek() const {
	return tokens[position];
}

Token Parser::previous() const {
	return tokens[position - 1];
}

Token Parser::advance() {
	if (!isAtEnd())
		position++;
	
	return previous();
}



