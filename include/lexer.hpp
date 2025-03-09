#pragma once

#include <string>
#include <vector>
#include <unordered_map>

class Token
{
public:
	enum class Kind
	{
		// Single-character tokens
		TOKEN_PLUS,
		TOKEN_MINUS,
		TOKEN_STAR,
		TOKEN_SLASH,
		TOKEN_COMMA,
		TOKEN_COLON,
		TOKEN_SEMI,
		TOKEN_EQUAL,
		TOKEN_AMPERSAND,
		TOKEN_LPAREN,
		TOKEN_RPAREN,
		TOKEN_LBRACE,
		TOKEN_RBRACE,
		TOKEN_LBRACKET,
		TOKEN_RBRACKET,

		// Multi-character tokens
		TOKEN_ARROW,
		TOKEN_FAT_ARROW,
		TOKEN_EQUAL_EQUAL,
		TOKEN_BANG_EQUAL,
		TOKEN_LESS_EQUAL,
		TOKEN_GREATER_EQUAL,
		TOKEN_LESS,
		TOKEN_GREATER,

		// Literals
		TOKEN_NUMBER,
		TOKEN_STRING,
		TOKEN_IDENTIFIER,

		// Keywords
		TOKEN_LET,
		TOKEN_FN,
		TOKEN_WHILE,
		TOKEN_IF,
		TOKEN_ELSE,
		TOKEN_RETURN,
		TOKEN_REF,
		TOKEN_EXTERN,
		TOKEN_INT_TYPE,

		// Special
		TOKEN_EOF,
		TOKEN_INVALID
	};

	Token(Kind kind, std::string value, int line)
		: kind(kind), value(std::move(value)), line(line) {}

	int getLine() const { return line; }
	Kind getKind() const { return kind; }
	const std::string &getValue() const { return value; }

private:
	int line;
	Kind kind;
	std::string value;
};

class Lexer
{
public:
	explicit Lexer(std::string source)
		: source(std::move(source)), position(0) {}

	std::vector<Token> tokenize();

private:
	Token nextToken();
	Token identifierOrKeyword();
	Token numberLiteral();
	Token stringLiteral();

	void skipWhitespaceAndComments();
	char advance();
	char peek() const;
	bool match(char expected);
	bool isIdentifierChar(char c) const;

	static const std::unordered_map<std::string, Token::Kind> keywords;
	static const std::unordered_map<char, Token::Kind> singleCharTokens;

	const std::string source;
	size_t position;
	int currentLine = 1;
};