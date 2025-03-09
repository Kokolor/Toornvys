#include <cctype>
#include <unordered_map>
#include "../include/error.hpp"
#include "../include/lexer.hpp"

const std::unordered_map<std::string, Token::Kind> Lexer::keywords = {
	{"let", Token::Kind::TOKEN_LET},
	{"fn", Token::Kind::TOKEN_FN},
	{"while", Token::Kind::TOKEN_WHILE},
	{"return", Token::Kind::TOKEN_RETURN},
	{"ref", Token::Kind::TOKEN_REF},
	{"ext", Token::Kind::TOKEN_EXTERN},
	{"i8", Token::Kind::TOKEN_INT_TYPE},
	{"i16", Token::Kind::TOKEN_INT_TYPE},
	{"i32", Token::Kind::TOKEN_INT_TYPE},
	{"i64", Token::Kind::TOKEN_INT_TYPE},
	{"void", Token::Kind::TOKEN_INT_TYPE}};

const std::unordered_map<char, Token::Kind> Lexer::singleCharTokens = {
	{'+', Token::Kind::TOKEN_PLUS},
	{'-', Token::Kind::TOKEN_MINUS},
	{'*', Token::Kind::TOKEN_STAR},
	{'/', Token::Kind::TOKEN_SLASH},
	{',', Token::Kind::TOKEN_COMMA},
	{':', Token::Kind::TOKEN_COLON},
	{';', Token::Kind::TOKEN_SEMI},
	{'=', Token::Kind::TOKEN_EQUAL},
	{'&', Token::Kind::TOKEN_AMPERSAND},
	{'(', Token::Kind::TOKEN_LPAREN},
	{')', Token::Kind::TOKEN_RPAREN},
	{'{', Token::Kind::TOKEN_LBRACE},
	{'}', Token::Kind::TOKEN_RBRACE},
	{'[', Token::Kind::TOKEN_LBRACKET},
	{']', Token::Kind::TOKEN_RBRACKET},
	{'<', Token::Kind::TOKEN_LESS},
	{'>', Token::Kind::TOKEN_GREATER}};

std::vector<Token> Lexer::tokenize()
{
	std::vector<Token> tokens;

	while (position < source.size())
	{
		skipWhitespaceAndComments();
		if (position >= source.size())
			break;

		tokens.push_back(nextToken());
	}

	tokens.emplace_back(Token::Kind::TOKEN_EOF, "EOF", currentLine);
	return tokens;
}

Token Lexer::nextToken()
{
	const char c = peek();

	if (c == '=' || c == '!' || c == '<' || c == '>' || c == '-')
	{
		const size_t start = position;
		advance();

		if (c == '=' && match('='))
			return Token(Token::Kind::TOKEN_EQUAL_EQUAL, "==", currentLine);
		if (c == '!' && match('='))
			return Token(Token::Kind::TOKEN_BANG_EQUAL, "!=", currentLine);
		if (c == '<' && match('='))
			return Token(Token::Kind::TOKEN_LESS_EQUAL, "<=", currentLine);
		if (c == '>' && match('='))
			return Token(Token::Kind::TOKEN_GREATER_EQUAL, ">=", currentLine);
		if (c == '-' && match('>'))
			return Token(Token::Kind::TOKEN_ARROW, "->", currentLine);

		position = start;
	}

	if (auto it = singleCharTokens.find(c); it != singleCharTokens.end())
	{
		advance();
		return Token(it->second, std::string(1, c), currentLine);
	}

	if (isalpha(c) || c == '_')
		return identifierOrKeyword();

	if (isdigit(c))
		return numberLiteral();

	if (c == '"')
		return stringLiteral();

	const std::string invalid(1, advance());
	ERROR(currentLine, "Invalid character: '%s'", invalid.c_str());
	return Token(Token::Kind::TOKEN_INVALID, invalid, currentLine);
}

Token Lexer::identifierOrKeyword()
{
	const size_t start = position;
	while (position < source.size() && isIdentifierChar(peek()))
		advance();

	const std::string value = source.substr(start, position - start);
	if (auto it = keywords.find(value); it != keywords.end())
	{
		return Token(it->second, value, currentLine);
	}

	return Token(Token::Kind::TOKEN_IDENTIFIER, value, currentLine);
}

Token Lexer::numberLiteral()
{
	const size_t start = position;
	bool hasDecimal = false;

	while (position < source.size())
	{
		const char c = peek();
		if (isdigit(c))
		{
			advance();
		}
		else if (c == '.' && !hasDecimal)
		{
			hasDecimal = true;
			advance();
		}
		else
		{
			break;
		}
	}

	return Token(Token::Kind::TOKEN_NUMBER, source.substr(start, position - start), currentLine);
}

Token Lexer::stringLiteral()
{
	advance();
	std::string value;

	while (position < source.size() && peek() != '"')
	{
		if (peek() == '\\')
		{
			advance();
			switch (advance())
			{
			case 'n':
				value += '\n';
				break;
			case 't':
				value += '\t';
				break;
			case '\\':
				value += '\\';
				break;
			case '"':
				value += '"';
				break;
			default:
				ERROR(currentLine, "Invalid escape sequence: \\%c", peek());
			}
		}
		else
		{
			if (peek() == '\n')
				currentLine++;
			value += advance();
		}
	}

	if (position >= source.size())
	{
		ERROR(currentLine, "Unterminated string literal");
	}
	advance();

	return Token(Token::Kind::TOKEN_STRING, value, currentLine);
}

void Lexer::skipWhitespaceAndComments()
{
	while (position < source.size())
	{
		const char c = peek();
		if (c == '\n')
		{
			currentLine++;
			advance();
		}
		else if (isspace(c))
		{
			advance();
		}
		else if (c == '/' && position + 1 < source.size() && source[position + 1] == '/')
		{
			while (position < source.size() && peek() != '\n')
				advance();
		}
		else
		{
			break;
		}
	}
}

char Lexer::advance()
{
	return source[position++];
}

char Lexer::peek() const
{
	return position < source.size() ? source[position] : '\0';
}

bool Lexer::match(char expected)
{
	if (position < source.size() && source[position] == expected)
	{
		advance();
		return true;
	}

	return false;
}

bool Lexer::isIdentifierChar(char c) const
{
	return isalnum(c) || c == '_';
}