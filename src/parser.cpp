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

	fprintf(stderr, "Unexpected token in factor: '%s'\n", peek().getValue().c_str());
	exit(EXIT_FAILURE);
}

std::unique_ptr<Node> Parser::parseStatement()
{
	if (matchSingleToken(Token::Kind::TOKEN_LET))
	{
		return parseVariableDeclaration();
	}
	else if (matchSingleToken(Token::Kind::TOKEN_FN))
	{
		return parseFuncDeclaration();
	}
	else if (matchSingleToken(Token::Kind::TOKEN_RETURN))
	{
		return parseReturn();
	}

	auto expr = parseAssignment();

	if (matchSingleToken(Token::Kind::TOKEN_SEMI))
	{
		advance();
	}

	return expr;
}

std::unique_ptr<NodeBlock> Parser::parseBlock()
{
	if (!matchSingleToken(Token::Kind::TOKEN_LBRACE))
	{
		fprintf(stderr, "Expected '{' at start of block");
	}

	advance();

	auto block = std::make_unique<NodeBlock>();

	while (!isAtEnd() && !matchSingleToken(Token::Kind::TOKEN_RBRACE))
	{
		block->addStatement(parseStatement());
	}

	if (!matchSingleToken(Token::Kind::TOKEN_RBRACE))
	{
		fprintf(stderr, "Expected '}' at end of block");
	}

	advance();

	return block;
}

std::unique_ptr<Node> Parser::parseReturn()
{
	advance();

	if (matchSingleToken(Token::Kind::TOKEN_SEMI))
	{
		advance();
		return std::make_unique<NodeReturn>(nullptr);
	}

	auto expr = parseExpression();

	if (matchSingleToken(Token::Kind::TOKEN_SEMI))
	{
		advance();
	}

	return std::make_unique<NodeReturn>(std::move(expr));
}

std::unique_ptr<Node> Parser::parseVariableDeclaration()
{
	advance();

	if (!matchSingleToken(Token::Kind::TOKEN_IDENTIFIER))
	{
		fprintf(stderr, "Expected variable name after 'let'\n");
		exit(EXIT_FAILURE);
	}

	advance();

	std::string name = previous().getValue();

	if (!matchSingleToken(Token::Kind::TOKEN_COLON))
	{
		fprintf(stderr, "Expected ':' after variable name\n");
		exit(EXIT_FAILURE);
	}

	advance();

	if (!matchSingleToken(Token::Kind::TOKEN_INT_TYPE))
	{
		fprintf(stderr, "Expected type after ':'\n");
		exit(EXIT_FAILURE);
	}

	advance();

	std::string type = previous().getValue();

	if (!matchSingleToken(Token::Kind::TOKEN_EQUAL))
	{
		fprintf(stderr, "Expected '=' after type\n");
		exit(EXIT_FAILURE);
	}

	advance();

	auto initializer = parseAssignment();

	if (matchSingleToken(Token::Kind::TOKEN_SEMI))
	{
		advance();
	}

	return std::make_unique<NodeVarDeclaration>(name, type, std::move(initializer));
}

std::unique_ptr<Node> Parser::parseFuncDeclaration()
{
	advance();

	if (!matchSingleToken(Token::Kind::TOKEN_IDENTIFIER))
	{
		fprintf(stderr, "Expected function name after 'fn'\n");
		exit(EXIT_FAILURE);
	}

	advance();

	std::string name = previous().getValue();

	if (!matchSingleToken(Token::Kind::TOKEN_LPAREN))
	{
		fprintf(stderr, "Expected '(' after function name\n");
		exit(EXIT_FAILURE);
	}

	std::vector<std::pair<std::string, std::string>> args;

	advance();

	while (!matchSingleToken(Token::Kind::TOKEN_RPAREN))
	{
		if (!matchSingleToken(Token::Kind::TOKEN_IDENTIFIER))
		{
			fprintf(stderr, "Expected argument name\n");
			exit(EXIT_FAILURE);
		}

		advance();

		std::string argName = previous().getValue();

		if (!matchSingleToken(Token::Kind::TOKEN_COLON))
		{
			fprintf(stderr, "Expected ':' after argument name\n");
			exit(EXIT_FAILURE);
		}

		advance();

		if (!matchSingleToken(Token::Kind::TOKEN_INT_TYPE))
		{
			fprintf(stderr, "Expected type after ':'\n");
			exit(EXIT_FAILURE);
		}

		advance();

		std::string argType = previous().getValue();

		args.emplace_back(argName, argType);

		if (matchSingleToken(Token::Kind::TOKEN_COMMA))
		{
			advance();
		}
	}

	advance();

	std::string returnType = "void";

	if (!matchSingleToken(Token::Kind::TOKEN_FAT_ARROW))
	{
		fprintf(stderr, "Expected '=>' before body\n");
		exit(EXIT_FAILURE);
	}

	advance();

	auto body = parseBlock();

	if (matchSingleToken(Token::Kind::TOKEN_ARROW))
	{
		advance();

		if (!matchSingleToken(Token::Kind::TOKEN_INT_TYPE))
		{
			fprintf(stderr, "Expected return type after ':'\n");
			exit(EXIT_FAILURE);
		}

		advance();

		returnType = previous().getValue();
	}

	return std::make_unique<NodeFuncDeclaration>(name, args, std::move(body), returnType);
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
