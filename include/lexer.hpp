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
		TOKEN_AMPERSAND,
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
		TOKEN_REF,
		TOKEN_EXTERN,

		TOKEN_INT_TYPE,
		TOKEN_STRING,

		TOKEN_EOF,
		TOKEN_INVALID
	};

	Token(Kind kind, const std::string &value, int line) : kind(kind), value(value), line(line) {}

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
	explicit Lexer(const std::string &source) : source(source), position(0) {}

	std::vector<Token> tokenize();

private:
	Token getNextToken(int line);
	Token getIdentifier(int line);
	Token getNumber(int line);
	Token getString(int line);

	bool isDigit(char character) const;
	bool isAlpha(char character) const;

	const std::string source;
	size_t position;
};
