#pragma once

#include <string>
#include <vector>

class Token
{
public:
	enum class Kind
	{
		TOKEN_NUMBER,
		TOKEN_IDENTIFIER,
		TOKEN_PLUS,
		TOKEN_MINUS,
		TOKEN_STAR,
		TOKEN_SLASH,
		TOKEN_COMMA,
		TOKEN_COLON,
		TOKEN_SEMI,
		TOKEN_EQUAL,
		TOKEN_ARROW,
		TOKEN_FAT_ARROW,

		TOKEN_EQUAL_EQUAL,	 // ==
		TOKEN_BANG_EQUAL,	 // !=
		TOKEN_LESS_EQUAL,	 // <=
		TOKEN_GREATER_EQUAL, // >=
		TOKEN_LESS,			 // <
		TOKEN_GREATER,		 // >

		TOKEN_LPAREN,
		TOKEN_RPAREN,
		TOKEN_LBRACE,
		TOKEN_RBRACE,

		TOKEN_LET,
		TOKEN_FN,
		TOKEN_RETURN,

		TOKEN_INT_TYPE,

		TOKEN_EOF,
		TOKEN_INVALID
	};

	Token(Kind kind, const std::string &value) : kind(kind), value(value) {}

	Kind getKind() const { return kind; }
	const std::string &getValue() const { return value; }

private:
	Kind kind;
	std::string value;
};

class Lexer
{
public:
	explicit Lexer(const std::string &source) : source(source), position(0) {}

	std::vector<Token> tokenize();

private:
	Token getNextToken();
	Token getIdentifier();
	Token getNumber();

	bool isDigit(char character) const;
	bool isAlpha(char character) const;

	const std::string source;
	size_t position;
};
