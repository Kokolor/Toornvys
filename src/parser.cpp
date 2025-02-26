#include <stdexcept>
#include "../include/parser.hpp"

std::unique_ptr<Node> Parser::parse()
{
	auto block = std::make_unique<NodeBlock>();

	while (!isAtEnd())
	{
		block->addStatement(parseStatement());
	}

	return block;
}

std::unique_ptr<Node> Parser::parseExpression()
{
	auto left = parseTerm();

	while (matchMultipleTokens({Token::Kind::TOKEN_PLUS, Token::Kind::TOKEN_MINUS}))
	{
		Token::Kind op = previous().getKind();
		auto right = parseTerm();
		left = std::make_unique<NodeBinaryOp>(op, std::move(left), std::move(right));
	}

	return left;
}

std::unique_ptr<Node> Parser::parseTerm()
{
	auto left = parseFactor();

	while (matchMultipleTokens({Token::Kind::TOKEN_STAR, Token::Kind::TOKEN_SLASH}))
	{
		Token::Kind op = previous().getKind();
		auto right = parseFactor();
		left = std::make_unique<NodeBinaryOp>(op, std::move(left), std::move(right));
	}

	return left;
}

std::unique_ptr<Node> Parser::parseFactor()
{
	if (matchSingleToken(Token::Kind::TOKEN_NUMBER))
	{
		advance();
		return std::make_unique<NodeNumber>(std::stoi(previous().getValue()));
	}

	if (matchSingleToken(Token::Kind::TOKEN_IDENTIFIER))
	{
		advance();
		return std::make_unique<NodeIdentifier>(previous().getValue());
	}

	fprintf(stderr, "Unexpected token in factor");
}

std::unique_ptr<Node> Parser::parseStatement()
{
	if (matchSingleToken(Token::Kind::TOKEN_LET))
	{
		return parseVariableDeclaration();
	}

	auto expr = parseAssignment();
	if (matchSingleToken(Token::Kind::TOKEN_SEMI))
	{
		advance();
	}

	return expr;
}

std::unique_ptr<Node> Parser::parseVariableDeclaration()
{
	advance();
	if (!matchSingleToken(Token::Kind::TOKEN_IDENTIFIER))
	{
		fprintf(stderr, "Expected variable name after 'let'");
	}

	std::string name = peek().getValue();
	advance();

	if (!matchSingleToken(Token::Kind::TOKEN_EQUAL))
	{
		fprintf(stderr, "Expected '=' after variable name");
	}

	advance();

	auto initializer = parseAssignment();
	if (matchSingleToken(Token::Kind::TOKEN_SEMI))
	{
		advance();
	}

	return std::make_unique<NodeVarDeclaration>(name, std::move(initializer));
}

std::unique_ptr<Node> Parser::parseAssignment()
{
	auto expr = parseExpression();

	if (matchSingleToken(Token::Kind::TOKEN_EQUAL))
	{
		advance();
		auto value = parseAssignment();
		
		if (auto ident = dynamic_cast<NodeIdentifier *>(expr.get()))
		{
			return std::make_unique<NodeAssignment>(ident->getName(), std::move(value));
		}
		else
		{
			fprintf(stderr, "Invalid assignment target.");
		}
	}

	return expr;
}

bool Parser::matchMultipleTokens(const std::vector<Token::Kind> &kinds)
{
	for (auto kind : kinds)
	{
		if (matchSingleToken(kind))
		{
			advance();

			return true;
		}
	}

	return false;
}

bool Parser::matchSingleToken(const Token::Kind kind)
{
	return !isAtEnd() && peek().getKind() == kind;
}

bool Parser::isAtEnd() const
{
	return peek().getKind() == Token::Kind::TOKEN_EOF;
}

Token Parser::peek() const
{
	return tokens[position];
}

Token Parser::previous() const
{
	return tokens[position - 1];
}

Token Parser::advance()
{
	if (!isAtEnd())
		position++;

	return previous();
}
