#include "../include/lexer.hpp"

std::vector<Token> Lexer::tokenize()
{
	std::vector<Token> tokens;

	while (position < source.size())
	{
		Token token = getNextToken();

		if (token.getKind() != Token::Kind::TOKEN_INVALID)
		{
			tokens.push_back(token);
		}
	}

	tokens.push_back(Token(Token::Kind::TOKEN_EOF, "EOF"));

	return tokens;
}

Token Lexer::getNextToken()
{
	while (position < source.size() && std::isspace(source[position]))
		position++;

	if (position >= source.size())
		return Token(Token::Kind::TOKEN_INVALID, "");

	char character = source[position];

	if (isAlpha(character))
	{
		return getIdentifier();
	}
	else if (isDigit(character))
	{
		return getNumber();
	}

	switch (character)
	{
	case '+':
		position++;
		return Token(Token::Kind::TOKEN_PLUS, "+");
	case '-':
		position++;

		if (position < source.size() && source[position] == '>')
		{
			position++;
			return Token(Token::Kind::TOKEN_ARROW, "->");
		}
		else
		{
			return Token(Token::Kind::TOKEN_MINUS, "-");
		}
	case '*':
		position++;
		return Token(Token::Kind::TOKEN_STAR, "*");
	case '/':
		position++;
		return Token(Token::Kind::TOKEN_SLASH, "/");
	case ',':
		position++;
		return Token(Token::Kind::TOKEN_COMMA, ",");
	case ':':
		position++;
		return Token(Token::Kind::TOKEN_COLON, ":");
	case ';':
		position++;
		return Token(Token::Kind::TOKEN_SEMI, ";");
	case '=':
		position++;

		if (position < source.size())
		{
			if (source[position] == '=')
			{
				position++;
				return Token(Token::Kind::TOKEN_EQUAL_EQUAL, "==");
			}
			else if (source[position] == '>')
			{
				position++;
				return Token(Token::Kind::TOKEN_FAT_ARROW, "=>");
			}
		}
		return Token(Token::Kind::TOKEN_EQUAL, "=");
	case '&':
		position++;
		return Token(Token::Kind::TOKEN_AMPERSAND, "&");
	case '<':
		position++;

		if (position < source.size() && source[position] == '=')
		{
			position++;
			return Token(Token::Kind::TOKEN_LESS_EQUAL, "<=");
		}
		else
		{
			return Token(Token::Kind::TOKEN_LESS, "<");
		}

	case '>':
		position++;

		if (position < source.size() && source[position] == '=')
		{
			position++;
			return Token(Token::Kind::TOKEN_GREATER_EQUAL, ">=");
		}
		else
		{
			return Token(Token::Kind::TOKEN_GREATER, ">");
		}

	case '!':
		position++;

		if (position < source.size() && source[position] == '=')
		{
			position++;
			return Token(Token::Kind::TOKEN_BANG_EQUAL, "!=");
		}
		else
		{
			return Token(Token::Kind::TOKEN_INVALID, "!");
		}
	case '(':
		position++;
		return Token(Token::Kind::TOKEN_LPAREN, "(");
	case ')':
		position++;
		return Token(Token::Kind::TOKEN_RPAREN, ")");
	case '{':
		position++;
		return Token(Token::Kind::TOKEN_LBRACE, "{");
	case '}':
		position++;
		return Token(Token::Kind::TOKEN_RBRACE, "}");
	default:
		position++;
		return Token(Token::Kind::TOKEN_INVALID, std::string(1, character));
	}
}

Token Lexer::getIdentifier()
{
	size_t startPos = position;

	while (position < source.size() && (isAlpha(source[position]) || isDigit(source[position])))
	{
		position++;
	}

	std::string identifier = source.substr(startPos, position - startPos);

	if (identifier == "let")
	{
		return Token(Token::Kind::TOKEN_LET, identifier);
	}
	else if (identifier == "fn")
	{
		return Token(Token::Kind::TOKEN_FN, identifier);
	}
	else if (identifier == "return")
	{
		return Token(Token::Kind::TOKEN_RETURN, identifier);
	}
	else if (identifier == "i8" || identifier == "i16" || identifier == "i32" || identifier == "i64")
	{
		return Token(Token::Kind::TOKEN_INT_TYPE, identifier);
	}

	return Token(Token::Kind::TOKEN_IDENTIFIER, identifier);
}

Token Lexer::getNumber()
{
	size_t startPos = position;
	bool hasDecimal = false;

	while (position < source.size() && (isDigit(source[position]) || (source[position] == '.' && !hasDecimal)))
	{
		if (source[position] == '.')
		{
			hasDecimal = true;
		}

		position++;
	}

	std::string number = source.substr(startPos, position - startPos);

	return Token(Token::Kind::TOKEN_NUMBER, number);
}

bool Lexer::isDigit(char character) const
{
	return std::isdigit(character);
}

bool Lexer::isAlpha(char character) const
{
	return std::isalpha(character) || character == '_';
}