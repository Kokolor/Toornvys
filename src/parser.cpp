#include <stdexcept>
#include "../include/error.hpp"
#include "../include/parser.hpp"

std::unique_ptr<Node> Parser::parse()
{
	auto block = std::make_unique<NodeBlock>(peek().getLine());

	while (!isAtEnd())
	{
		block->addStatement(parseStatement());
	}

	return block;
}

std::unique_ptr<Node> Parser::parseExpression()
{
	return parseComparison();
}

std::unique_ptr<Node> Parser::parseComparison()
{
	auto left = parseAdditive();

	while (matchMultipleTokens({Token::Kind::TOKEN_EQUAL_EQUAL,
								Token::Kind::TOKEN_BANG_EQUAL,
								Token::Kind::TOKEN_LESS_EQUAL,
								Token::Kind::TOKEN_GREATER_EQUAL,
								Token::Kind::TOKEN_LESS,
								Token::Kind::TOKEN_GREATER}))
	{
		Token::Kind op = previous().getKind();
		auto right = parseAdditive();
		left = std::make_unique<NodeBinaryOp>(op, std::move(left), std::move(right), previous().getLine());
	}

	return left;
}

std::unique_ptr<Node> Parser::parseAdditive()
{
	auto left = parseTerm();

	while (matchMultipleTokens({Token::Kind::TOKEN_PLUS, Token::Kind::TOKEN_MINUS}))
	{
		Token::Kind op = previous().getKind();
		auto right = parseTerm();
		left = std::make_unique<NodeBinaryOp>(op, std::move(left), std::move(right), previous().getLine());
	}

	return left;
}

std::unique_ptr<Node> Parser::parsePrimary()
{
	int line = peek().getLine();

	if (matchSingleToken(Token::Kind::TOKEN_NUMBER))
	{
		advance();

		const std::string &numStr = previous().getValue();
		line = previous().getLine();
		char *end;
		long num = strtol(numStr.c_str(), &end, 10);

		if (end != numStr.c_str() + numStr.size())
		{
			ERROR(line, "Invalid integer: %s", numStr.c_str());
		}

		return std::make_unique<NodeNumber>(static_cast<int>(num), line);
	}
	if (matchSingleToken(Token::Kind::TOKEN_IDENTIFIER))
	{
		advance();

		std::string name = previous().getValue();
		int currentLine = previous().getLine();

		if (matchSingleToken(Token::Kind::TOKEN_LPAREN))
		{
			advance();

			std::vector<std::unique_ptr<Node>> args;

			while (!matchSingleToken(Token::Kind::TOKEN_RPAREN))
			{
				args.push_back(parseExpression());

				if (!matchSingleToken(Token::Kind::TOKEN_COMMA))
					break;

				advance();
			}

			if (!matchSingleToken(Token::Kind::TOKEN_RPAREN))
			{
				ERROR(peek().getLine(), "Expected ')' after function arguments");
			}

			advance();

			return std::make_unique<NodeFunctionCall>(name, std::move(args), currentLine);
		}
		else
		{
			return std::make_unique<NodeIdentifier>(name, currentLine);
		}
	}
	if (matchSingleToken(Token::Kind::TOKEN_LPAREN))
	{
		advance();

		auto expr = parseExpression();

		if (!matchSingleToken(Token::Kind::TOKEN_RPAREN))
		{
			ERROR(peek().getLine(), "Expected ')' after expression");
		}

		advance();

		return expr;
	}

	ERROR(peek().getLine(), "Unexpected token in primary: '%s'", peek().getValue().c_str());
}

std::unique_ptr<Node> Parser::parseUnary()
{
	if (matchSingleToken(Token::Kind::TOKEN_STAR))
	{
		advance();
		return std::make_unique<NodeUnaryOp>(Token::Kind::TOKEN_STAR, parseUnary(), previous().getLine());
	}
	else if (matchSingleToken(Token::Kind::TOKEN_AMPERSAND))
	{
		advance();
		return std::make_unique<NodeUnaryOp>(Token::Kind::TOKEN_AMPERSAND, parseUnary(), previous().getLine());
	}

	return parsePrimary();
}

std::unique_ptr<Node> Parser::parseTerm()
{
	auto left = parseFactor();

	while (matchMultipleTokens({Token::Kind::TOKEN_STAR, Token::Kind::TOKEN_SLASH}))
	{
		Token::Kind op = previous().getKind();
		auto right = parseFactor();
		left = std::make_unique<NodeBinaryOp>(op, std::move(left), std::move(right), previous().getLine());
	}

	return left;
}

std::unique_ptr<Node> Parser::parseFactor()
{
	return parseUnary();
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
		ERROR(peek().getLine(), "Expected '{' at start of block");
	}

	advance();

	auto block = std::make_unique<NodeBlock>(previous().getLine());

	while (!isAtEnd() && !matchSingleToken(Token::Kind::TOKEN_RBRACE))
	{
		block->addStatement(parseStatement());
	}

	if (!matchSingleToken(Token::Kind::TOKEN_RBRACE))
	{
		ERROR(peek().getLine(), "Expected '}' at end of block");
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
		return std::make_unique<NodeReturn>(nullptr, previous().getLine());
	}

	auto expr = parseExpression();

	if (matchSingleToken(Token::Kind::TOKEN_SEMI))
	{
		advance();
	}

	return std::make_unique<NodeReturn>(std::move(expr), previous().getLine());
}

std::unique_ptr<Node> Parser::parseVariableDeclaration()
{
	advance();

	if (!matchSingleToken(Token::Kind::TOKEN_IDENTIFIER))
	{
		ERROR(peek().getLine(), "Expected variable name after 'let'\n");
		exit(EXIT_FAILURE);
	}

	advance();

	std::string name = previous().getValue();

	if (!matchSingleToken(Token::Kind::TOKEN_COLON))
	{
		ERROR(peek().getLine(), "Expected ':' after variable name\n");
		exit(EXIT_FAILURE);
	}

	advance();

	std::string typeStr;

	if (matchSingleToken(Token::Kind::TOKEN_INT_TYPE))
	{
		advance();
		typeStr = previous().getValue();
	}
	else
	{
		ERROR(peek().getLine(), "Expected base type after ':'\n");
		exit(EXIT_FAILURE);
	}

	while (matchSingleToken(Token::Kind::TOKEN_STAR))
	{
		advance();
		typeStr += "*";
	}

	if (!matchSingleToken(Token::Kind::TOKEN_EQUAL))
	{
		ERROR(peek().getLine(), "Expected '=' after type\n");
		exit(EXIT_FAILURE);
	}

	advance();

	auto initializer = parseAssignment();

	if (matchSingleToken(Token::Kind::TOKEN_SEMI))
	{
		advance();
	}

	return std::make_unique<NodeVarDeclaration>(name, typeStr, std::move(initializer), previous().getLine());
}

std::unique_ptr<Node> Parser::parseFuncDeclaration()
{
	advance();

	if (!matchSingleToken(Token::Kind::TOKEN_IDENTIFIER))
	{
		ERROR(peek().getLine(), "Expected function name after 'fn'\n");
		exit(EXIT_FAILURE);
	}

	advance();

	std::string name = previous().getValue();

	if (!matchSingleToken(Token::Kind::TOKEN_LPAREN))
	{
		ERROR(peek().getLine(), "Expected '(' after function name\n");
		exit(EXIT_FAILURE);
	}

	std::vector<std::pair<std::string, std::string>> args;

	advance();

	while (!matchSingleToken(Token::Kind::TOKEN_RPAREN))
	{
		if (!matchSingleToken(Token::Kind::TOKEN_IDENTIFIER))
		{
			ERROR(peek().getLine(), "Expected argument name\n");
			exit(EXIT_FAILURE);
		}

		advance();

		std::string argName = previous().getValue();

		if (!matchSingleToken(Token::Kind::TOKEN_COLON))
		{
			ERROR(peek().getLine(), "Expected ':' after argument name\n");
			exit(EXIT_FAILURE);
		}

		advance();

		if (!matchSingleToken(Token::Kind::TOKEN_INT_TYPE))
		{
			ERROR(peek().getLine(), "Expected type after ':'\n");
			exit(EXIT_FAILURE);
		}

		advance();

		std::string argType = previous().getValue();

		while (matchSingleToken(Token::Kind::TOKEN_STAR))
		{
			advance();
			argType += "*";
		}

		args.emplace_back(argName, argType);

		if (matchSingleToken(Token::Kind::TOKEN_COMMA))
		{
			advance();
		}
	}

	advance();

	std::string returnType = "void";

	if (!matchSingleToken(Token::Kind::TOKEN_ARROW))
	{
		ERROR(peek().getLine(), "Expected '=>' before type\n");
		exit(EXIT_FAILURE);
	}

	advance();

	if (!matchSingleToken(Token::Kind::TOKEN_INT_TYPE))
	{
		ERROR(peek().getLine(), "Expected return type after '->'\n");
		exit(EXIT_FAILURE);
	}

	advance();

	std::string retType = previous().getValue();

	while (matchSingleToken(Token::Kind::TOKEN_STAR))
	{
		advance();
		retType += "*";
	}

	returnType = retType;

	auto body = parseBlock();

	return std::make_unique<NodeFuncDeclaration>(name, args, std::move(body), returnType, previous().getLine());
}

std::unique_ptr<Node> Parser::parseAssignment()
{
	auto expr = parseUnary();

	if (matchSingleToken(Token::Kind::TOKEN_EQUAL))
	{
		advance();
		
		auto value = parseAssignment();

		if (auto ident = dynamic_cast<NodeIdentifier *>(expr.get()))
		{
			return std::make_unique<NodeAssignment>(ident->getName(), std::move(value), ident->getLine());
		}
		else if (auto unary = dynamic_cast<NodeUnaryOp *>(expr.get()))
		{
			if (unary->getOp() == Token::Kind::TOKEN_STAR)
			{
				return std::make_unique<NodePointerAssignment>(std::move(unary->operand), std::move(value), unary->getLine());
			}
		}

		ERROR(expr->getLine(), "Cible d'assignation invalide");
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
