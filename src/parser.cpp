#include <stdexcept>
#include "../include/error.hpp"
#include "../include/parser.hpp"

const std::vector<Token::Kind> COMPARISON_OPS = {
	Token::Kind::TOKEN_EQUAL_EQUAL,
	Token::Kind::TOKEN_BANG_EQUAL,
	Token::Kind::TOKEN_LESS_EQUAL,
	Token::Kind::TOKEN_GREATER_EQUAL,
	Token::Kind::TOKEN_LESS,
	Token::Kind::TOKEN_GREATER};

const std::vector<Token::Kind> ADDITIVE_OPS = {
	Token::Kind::TOKEN_PLUS,
	Token::Kind::TOKEN_MINUS};

const std::vector<Token::Kind> MULTIPLICATIVE_OPS = {
	Token::Kind::TOKEN_STAR,
	Token::Kind::TOKEN_SLASH};

std::unique_ptr<Node> Parser::parse()
{
	auto block = std::make_unique<NodeBlock>(peek().getLine());
	while (!isAtEnd())
		block->addStatement(parseStatement());
	return block;
}

std::unique_ptr<Node> Parser::parseExpression()
{
	return parseComparison();
}

std::unique_ptr<Node> Parser::parseComparison()
{
	auto left = parseAdditive();

	while (matchMultipleTokens(COMPARISON_OPS))
	{
		const Token::Kind op = previous().getKind();
		const int line = previous().getLine();
		auto right = parseAdditive();
		left = std::make_unique<NodeBinaryOp>(op, std::move(left), std::move(right), line);
	}

	return left;
}

std::unique_ptr<Node> Parser::parseAdditive()
{
	auto left = parseTerm();

	while (matchMultipleTokens(ADDITIVE_OPS))
	{
		const Token::Kind op = previous().getKind();
		const int line = previous().getLine();
		auto right = parseTerm();
		left = std::make_unique<NodeBinaryOp>(op, std::move(left), std::move(right), line);
	}

	return left;
}

std::unique_ptr<Node> Parser::parseTerm()
{
	auto left = parseFactor();

	while (matchMultipleTokens(MULTIPLICATIVE_OPS))
	{
		const Token::Kind op = previous().getKind();
		const int line = previous().getLine();
		auto right = parseFactor();
		left = std::make_unique<NodeBinaryOp>(op, std::move(left), std::move(right), line);
	}

	return left;
}

std::unique_ptr<Node> Parser::parseFactor()
{
	return parseUnary();
}

std::unique_ptr<Node> Parser::parseUnary()
{
	if (matchSingleToken(Token::Kind::TOKEN_STAR) || matchSingleToken(Token::Kind::TOKEN_AMPERSAND))
	{
		const Token::Kind op = peek().getKind();
		consumeToken();
		return std::make_unique<NodeUnaryOp>(op, parseUnary(), previous().getLine());
	}

	return parsePrimary();
}

std::unique_ptr<Node> Parser::parsePrimary()
{
	const int line = peek().getLine();

	if (matchSingleToken(Token::Kind::TOKEN_NUMBER))
		return parseNumberLiteral();
	if (matchSingleToken(Token::Kind::TOKEN_STRING))
		return parseStringLiteral();
	if (matchSingleToken(Token::Kind::TOKEN_IDENTIFIER))
		return parseIdentifierExpression();
	if (matchSingleToken(Token::Kind::TOKEN_LPAREN))
		return parseParenthesizedExpression();

	ERROR(line, "Unexpected token in primary: '%s'", peek().getValue().c_str());
}

std::unique_ptr<Node> Parser::parseNumberLiteral()
{
	const Token numToken = consumeToken();
	char *end;
	const std::string &numStr = numToken.getValue();
	const long num = strtol(numStr.c_str(), &end, 10);

	if (end != numStr.c_str() + numStr.size())
	{
		ERROR(numToken.getLine(), "Invalid integer: %s", numStr.c_str());
	}

	return std::make_unique<NodeNumber>(static_cast<int>(num), numToken.getLine());
}

std::unique_ptr<Node> Parser::parseStringLiteral()
{
	const Token strToken = consumeToken();
	return std::make_unique<NodeString>(strToken.getValue(), strToken.getLine());
}

std::unique_ptr<Node> Parser::parseIdentifierExpression()
{
	const Token identToken = consumeToken();
	const std::string name = identToken.getValue();
	const int line = identToken.getLine();

	if (matchSingleToken(Token::Kind::TOKEN_LPAREN))
	{
		return parseFunctionCall(name, line);
	}
	if (matchSingleToken(Token::Kind::TOKEN_LBRACKET))
	{
		return parseArrayAccess(name, line);
	}

	return std::make_unique<NodeIdentifier>(name, line);
}

std::unique_ptr<Node> Parser::parseParenthesizedExpression()
{
	consumeToken(Token::Kind::TOKEN_LPAREN, "Expected '('");
	auto expr = parseExpression();

	if (matchSingleToken(Token::Kind::TOKEN_ARROW))
	{
		return parseCastOperation(std::move(expr));
	}

	consumeToken(Token::Kind::TOKEN_RPAREN, "Expected ')' after expression");
	return expr;
}

std::unique_ptr<Node> Parser::parseCastOperation(std::unique_ptr<Node> expr)
{
	consumeToken(Token::Kind::TOKEN_ARROW, "Expected '->'");
	const std::string targetType = parseType();
	consumeToken(Token::Kind::TOKEN_RPAREN, "Expected ')' after cast");
	return std::make_unique<NodeCast>(targetType, std::move(expr), expr->getLine());
}

std::unique_ptr<Node> Parser::parseFunctionCall(const std::string &name, int line)
{
	consumeToken(Token::Kind::TOKEN_LPAREN, "Expected '('");
	std::vector<std::unique_ptr<Node>> args;

	while (!matchSingleToken(Token::Kind::TOKEN_RPAREN))
	{
		args.push_back(parseExpression());
		if (!matchSingleToken(Token::Kind::TOKEN_COMMA))
			break;
		consumeToken();
	}

	consumeToken(Token::Kind::TOKEN_RPAREN, "Expected ')'");
	return std::make_unique<NodeFunctionCall>(name, std::move(args), line);
}

std::unique_ptr<Node> Parser::parseArrayAccess(const std::string &name, int line)
{
	consumeToken(Token::Kind::TOKEN_LBRACKET, "Expected '['");
	auto index = parseExpression();
	consumeToken(Token::Kind::TOKEN_RBRACKET, "Expected ']'");
	return std::make_unique<NodeArrayAccess>(name, std::move(index), line);
}

std::string Parser::parseType()
{
	std::string typeStr;
	consumeToken(Token::Kind::TOKEN_INT_TYPE, "Expected base type");
	typeStr = previous().getValue();

	while (matchSingleToken(Token::Kind::TOKEN_STAR))
	{
		consumeToken();
		typeStr += "*";
	}

	while (matchSingleToken(Token::Kind::TOKEN_LBRACKET))
	{
		consumeToken();
		consumeToken(Token::Kind::TOKEN_NUMBER, "Expected array size");
		typeStr += "[" + previous().getValue() + "]";
		consumeToken(Token::Kind::TOKEN_RBRACKET, "Expected ']'");
	}

	return typeStr;
}

std::vector<std::pair<std::string, std::string>> Parser::parseArgumentList()
{
	std::vector<std::pair<std::string, std::string>> args;
	consumeToken(Token::Kind::TOKEN_LPAREN, "Expected '('");

	while (!matchSingleToken(Token::Kind::TOKEN_RPAREN))
	{
		args.push_back(parseArgument());
		if (!matchSingleToken(Token::Kind::TOKEN_COMMA))
			break;
		consumeToken();
	}

	consumeToken(Token::Kind::TOKEN_RPAREN, "Expected ')'");
	return args;
}

std::pair<std::string, std::string> Parser::parseArgument()
{
	const std::string argName = consumeToken(Token::Kind::TOKEN_IDENTIFIER, "Expected argument name").getValue();
	consumeToken(Token::Kind::TOKEN_COLON, "Expected ':'");
	return {argName, parseType()};
}

std::unique_ptr<Node> Parser::parseStatement()
{
	if (matchSingleToken(Token::Kind::TOKEN_LET))
		return parseVariableDeclaration();
	if (matchSingleToken(Token::Kind::TOKEN_FN))
		return parseFunctionDeclaration();
	if (matchSingleToken(Token::Kind::TOKEN_RETURN))
		return parseReturn();
	if (matchSingleToken(Token::Kind::TOKEN_EXTERN))
		return parseExternDeclaration();

	auto expr = parseAssignment();
	if (matchSingleToken(Token::Kind::TOKEN_SEMI))
		consumeToken();

	return expr;
}

std::unique_ptr<NodeBlock> Parser::parseBlock()
{
	consumeToken(Token::Kind::TOKEN_LBRACE, "Expected '{'");
	auto block = std::make_unique<NodeBlock>(previous().getLine());

	while (!isAtEnd() && !matchSingleToken(Token::Kind::TOKEN_RBRACE))
	{
		block->addStatement(parseStatement());
	}

	consumeToken(Token::Kind::TOKEN_RBRACE, "Expected '}'");
	return block;
}

std::unique_ptr<Node> Parser::parseReturn()
{
	const int line = consumeToken(Token::Kind::TOKEN_RETURN, "Unexpected return").getLine();
	std::unique_ptr<Node> expr = nullptr;

	if (!matchSingleToken(Token::Kind::TOKEN_SEMI))
	{
		expr = parseExpression();
	}
	if (matchSingleToken(Token::Kind::TOKEN_SEMI))
		consumeToken();

	return std::make_unique<NodeReturn>(std::move(expr), line);
}

std::unique_ptr<Node> Parser::parseVariableDeclaration()
{
	consumeToken(Token::Kind::TOKEN_LET, "Unexpected let");
	const std::string name = consumeToken(Token::Kind::TOKEN_IDENTIFIER, "Expected variable name").getValue();
	consumeToken(Token::Kind::TOKEN_COLON, "Expected ':'");

	const std::string typeStr = parseType();
	std::unique_ptr<Node> initializer = nullptr;

	if (matchSingleToken(Token::Kind::TOKEN_EQUAL))
	{
		consumeToken();
		initializer = parseAssignment();
	}

	if (matchSingleToken(Token::Kind::TOKEN_SEMI))
		consumeToken();

	return std::make_unique<NodeVariableDeclaration>(name, typeStr, std::move(initializer), previous().getLine());
}

std::unique_ptr<Node> Parser::parseFunctionDeclaration()
{
	consumeToken(Token::Kind::TOKEN_FN, "Unexpected fn");
	const std::string name = consumeToken(Token::Kind::TOKEN_IDENTIFIER, "Expected function name").getValue();

	const auto args = parseArgumentList();
	consumeToken(Token::Kind::TOKEN_ARROW, "Expected '->'");

	const std::string returnType = parseType();
	auto body = parseBlock();
	return std::make_unique<NodeFunctionDeclaration>(name, args, std::move(body), returnType, previous().getLine());
}

std::unique_ptr<Node> Parser::parseExternDeclaration()
{
	consumeToken(Token::Kind::TOKEN_EXTERN, "Unexpected extern");
	const std::string name = consumeToken(Token::Kind::TOKEN_IDENTIFIER, "Expected function name").getValue();

	const auto args = parseArgumentList();
	consumeToken(Token::Kind::TOKEN_ARROW, "Expected '->'");

	const std::string returnType = parseType();
	consumeToken(Token::Kind::TOKEN_SEMI, "Expected ';'");
	return std::make_unique<NodeExternDeclaration>(name, args, returnType, previous().getLine());
}

std::unique_ptr<Node> Parser::parseAssignment()
{
	auto expr = parseComparison();

	if (matchSingleToken(Token::Kind::TOKEN_EQUAL))
	{
		consumeToken();
		auto value = parseAssignment();

		if (auto *ident = dynamic_cast<NodeIdentifier *>(expr.get()))
		{
			return std::make_unique<NodeAssignment>(ident->getName(), std::move(value), ident->getLine());
		}
		else if (auto *unary = dynamic_cast<NodeUnaryOp *>(expr.get()))
		{
			if (unary->getOp() == Token::Kind::TOKEN_STAR)
			{
				return std::make_unique<NodePointerAssignment>(
					std::move(unary->operand), std::move(value), unary->getLine());
			}
		}
		else if (auto *arrayAccess = dynamic_cast<NodeArrayAccess *>(expr.get()))
		{
			return std::make_unique<NodeArrayAssignment>(
				arrayAccess->getName(),
				arrayAccess->releaseIndex(),
				std::move(value),
				arrayAccess->getLine());
		}

		ERROR(expr->getLine(), "Invalid assignment target");
	}

	return expr;
}

bool Parser::matchMultipleTokens(const std::vector<Token::Kind> &kinds)
{
	for (auto kind : kinds)
	{
		if (matchSingleToken(kind))
		{
			consumeToken();
			return true;
		}
	}

	return false;
}

bool Parser::matchSingleToken(const Token::Kind kind)
{
	return !isAtEnd() && peek().getKind() == kind;
}

Token Parser::consumeToken(Token::Kind expected, const std::string &errorMsg)
{
	if (!matchSingleToken(expected))
		ERROR(peek().getLine(), "%s", errorMsg.c_str());
	return advance();
}

Token Parser::consumeToken()
{
	if (isAtEnd())
		ERROR(peek().getLine(), "Unexpected end of input");
	return advance();
}

Token Parser::advance()
{
	if (!isAtEnd())
		position++;
	return previous();
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
	return tokens[position > 0 ? position - 1 : 0];
}