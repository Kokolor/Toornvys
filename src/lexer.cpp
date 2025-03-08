#include "../include/error.hpp"
#include "../include/lexer.hpp"

std::vector<Token> Lexer::tokenize()
{
	std::vector<Token> tokens;
	int line = 1;

	while (position < source.size())
	{
		while (position < source.size() && std::isspace(source[position]))
		{
			if (source[position] == '\n')
				line++;
			position++;
		}

		if (position >= source.size())
			break;

		Token token = getNextToken(line);

		if (token.getKind() != Token::Kind::TOKEN_INVALID)
		{
			tokens.push_back(token);
		}
	}

	tokens.push_back(Token(Token::Kind::TOKEN_EOF, "EOF", line));

	return tokens;
}

Token Lexer::getNextToken(int line)
{
	while (position < source.size() && std::isspace(source[position]))
	{
		if (source[position] == '\n')
			line++;

		position++;
	}

	if (position >= source.size())
		return Token(Token::Kind::TOKEN_INVALID, "", line);

	char character = source[position];

	if (isAlpha(character))
	{
		return getIdentifier(line);
	}
	else if (isDigit(character))
	{
		return getNumber(line);
	}
	else if (character == '"')
	{
		return getString(line);
	}

	switch (character)
	{
	case '+':
		position++;
		return Token(Token::Kind::TOKEN_PLUS, "+", line);
	case '-':
		position++;

		if (position < source.size() && source[position] == '>')
		{
			position++;
			return Token(Token::Kind::TOKEN_ARROW, "->", line);
		}
		else
		{
			return Token(Token::Kind::TOKEN_MINUS, "-", line);
		}
	case '*':
		position++;
		return Token(Token::Kind::TOKEN_STAR, "*", line);
	case '/':
		position++;
		return Token(Token::Kind::TOKEN_SLASH, "/", line);
	case ',':
		position++;
		return Token(Token::Kind::TOKEN_COMMA, ",", line);
	case ':':
		position++;
		return Token(Token::Kind::TOKEN_COLON, ":", line);
	case ';':
		position++;
		return Token(Token::Kind::TOKEN_SEMI, ";", line);
	case '=':
		position++;

		if (position < source.size())
		{
			if (source[position] == '=')
			{
				position++;
				return Token(Token::Kind::TOKEN_EQUAL_EQUAL, "==", line);
			}
			else if (source[position] == '>')
			{
				position++;
				return Token(Token::Kind::TOKEN_FAT_ARROW, "=>", line);
			}
		}
		return Token(Token::Kind::TOKEN_EQUAL, "=", line);
	case '&':
		position++;
		return Token(Token::Kind::TOKEN_AMPERSAND, "&", line);
	case '<':
		position++;

		if (position < source.size() && source[position] == '=')
		{
			position++;
			return Token(Token::Kind::TOKEN_LESS_EQUAL, "<=", line);
		}
		else
		{
			return Token(Token::Kind::TOKEN_LESS, "<", line);
		}

	case '>':
		position++;

		if (position < source.size() && source[position] == '=')
		{
			position++;
			return Token(Token::Kind::TOKEN_GREATER_EQUAL, ">=", line);
		}
		else
		{
			return Token(Token::Kind::TOKEN_GREATER, ">", line);
		}

	case '!':
		position++;

		if (position < source.size() && source[position] == '=')
		{
			position++;
			return Token(Token::Kind::TOKEN_BANG_EQUAL, "!=", line);
		}
		else
		{
			return Token(Token::Kind::TOKEN_INVALID, "!", line);
		}
	case '(':
		position++;
		return Token(Token::Kind::TOKEN_LPAREN, "(", line);
	case ')':
		position++;
		return Token(Token::Kind::TOKEN_RPAREN, ")", line);
	case '{':
		position++;
		return Token(Token::Kind::TOKEN_LBRACE, "{", line);
	case '}':
		position++;
		return Token(Token::Kind::TOKEN_RBRACE, "}", line);
	default:
		position++;
		return Token(Token::Kind::TOKEN_INVALID, std::string(1, character), line);
	}
}

Token Lexer::getIdentifier(int line)
{
	size_t startPos = position;

	while (position < source.size() && (isAlpha(source[position]) || isDigit(source[position])))
	{
		if (source[position] == '\n')
			line++;

		position++;
	}

	std::string identifier = source.substr(startPos, position - startPos);

	if (identifier == "let")
	{
		return Token(Token::Kind::TOKEN_LET, identifier, line);
	}
	else if (identifier == "fn")
	{
		return Token(Token::Kind::TOKEN_FN, identifier, line);
	}
	else if (identifier == "return")
	{
		return Token(Token::Kind::TOKEN_RETURN, identifier, line);
	}
	else if (identifier == "ref")
	{
		return Token(Token::Kind::TOKEN_REF, identifier, line);
	}
	else if (identifier == "ext")
	{
		return Token(Token::Kind::TOKEN_EXTERN, identifier, line);
	}
	else if (identifier == "i8" || identifier == "i16" || identifier == "i32" || identifier == "i64" || identifier == "void")
	{
		return Token(Token::Kind::TOKEN_INT_TYPE, identifier, line);
	}

	return Token(Token::Kind::TOKEN_IDENTIFIER, identifier, line);
}

Token Lexer::getNumber(int line)
{
	size_t startPos = position;
	bool hasDecimal = false;

	while (position < source.size() && (isDigit(source[position]) || (source[position] == '.' && !hasDecimal)))
	{
		if (source[position] == '\n')
			line++;

		if (source[position] == '.')
		{
			hasDecimal = true;
		}

		position++;
	}

	std::string number = source.substr(startPos, position - startPos);

	return Token(Token::Kind::TOKEN_NUMBER, number, line);
}

Token Lexer::getString(int line)
{
	position++;
	
	std::string str;

	while (position < source.size() && source[position] != '"')
	{
		if (source[position] == '\\')
		{
			position++;

			if (position >= source.size())
			{
				ERROR(line, "Unterminated escape sequence in string literal");
			}

			switch (source[position])
			{
			case 'n':
				str += '\n';
				break;
			case 't':
				str += '\t';
				break;
			case '\\':
				str += '\\';
				break;
			case '"':
				str += '"';
				break;
			default:
				ERROR(line, "Unknown escape sequence: \\%c", source[position]);
			}
		}
		else
		{
			str += source[position];
		}

		if (source[position] == '\n')
		{
			line++;
		}

		position++;
	}

	if (position >= source.size())
	{
		ERROR(line, "Unterminated string literal");
	}

	position++;

	return Token(Token::Kind::TOKEN_STRING, str, line);
}

bool Lexer::isDigit(char character) const
{
	return std::isdigit(character);
}

bool Lexer::isAlpha(char character) const
{
	return std::isalpha(character) || character == '_';
}