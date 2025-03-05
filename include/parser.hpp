#pragma once

#include <memory>
#include "lexer.hpp"

class Node
{
public:
	virtual ~Node() = default;

	virtual int getLine() const = 0;
};

class NodeNumber : public Node
{
public:
	explicit NodeNumber(int value, int line) : value(value), line(line) {}

	int getValue() const { return value; }
	int getLine() const override { return line; }

private:
	int value;
	int line;
};

class NodeIdentifier : public Node
{
public:
	explicit NodeIdentifier(const std::string &name, int line) : name(name), line(line) {}

	const std::string &getName() const { return name; }
	int getLine() const override { return line; }

private:
	std::string name;
	int line;
};

class NodeBinaryOp : public Node
{
public:
	NodeBinaryOp(Token::Kind op, std::unique_ptr<Node> left, std::unique_ptr<Node> right, int line) : op(op), left(std::move(left)), right(std::move(right)), line(line) {}

	Token::Kind getOp() const { return op; }
	const Node *getLeft() const { return left.get(); }
	const Node *getRight() const { return right.get(); }
	int getLine() const override { return line; }

private:
	Token::Kind op;
	std::unique_ptr<Node> left;
	std::unique_ptr<Node> right;
	int line;
};

class NodeUnaryOp : public Node
{
public:
	NodeUnaryOp(Token::Kind op, std::unique_ptr<Node> operand, int line) : op(op), operand(std::move(operand)), line(line) {}

	Token::Kind getOp() const { return op; }
	const Node *getOperand() const { return operand.get(); }
	int getLine() const override { return line; }

private:
	Token::Kind op;
	std::unique_ptr<Node> operand;
	int line;
};

class NodeReturn : public Node
{
public:
	explicit NodeReturn(std::unique_ptr<Node> expression, int line) : expression(std::move(expression)), line(line) {}

	const Node *getExpression() const { return expression.get(); }
	int getLine() const override { return line; }

private:
	std::unique_ptr<Node> expression;
	int line;
};

class NodeVarDeclaration : public Node
{
public:
	NodeVarDeclaration(const std::string &name, std::string &type, std::unique_ptr<Node> initializer, int line) : name(name), type(type), initializer(std::move(initializer)), line(line) {}

	const std::string &getName() const { return name; }
	const std::string &getType() const { return type; }
	const Node *getInitializer() const { return initializer.get(); }
	int getLine() const override { return line; }

private:
	std::string name;
	std::string type;
	std::unique_ptr<Node> initializer;
	int line;
};

class NodeFuncDeclaration : public Node
{
public:
	NodeFuncDeclaration(const std::string &name, const std::vector<std::pair<std::string, std::string>> &args, std::unique_ptr<Node> body, const std::string &returnType, int line) : name(name), args(args), body(std::move(body)), returnType(returnType), line(line) {}

	const std::string &getName() const { return name; }
	const std::vector<std::pair<std::string, std::string>> &getArgs() const { return args; }
	const std::unique_ptr<Node> &getBody() const { return body; }
	const std::string &getReturnType() const { return returnType; }
	int getLine() const override { return line; }

private:
	std::string name;
	std::vector<std::pair<std::string, std::string>> args;
	std::unique_ptr<Node> body;
	std::string returnType;
	int line;
};

class NodeFunctionCall : public Node
{
public:
	NodeFunctionCall(const std::string &name, std::vector<std::unique_ptr<Node>> args, int line) : name(name), args(std::move(args)), line(line) {}

	const std::string &getName() const { return name; }
	const std::vector<std::unique_ptr<Node>> &getArgs() const { return args; }
	int getLine() const override { return line; }

private:
	std::string name;
	std::vector<std::unique_ptr<Node>> args;
	int line;
};

class NodeAssignment : public Node
{
public:
	NodeAssignment(const std::string &name, std::unique_ptr<Node> value, int line) : name(name), value(std::move(value)), line(line) {}

	const std::string &getName() const { return name; }
	const Node *getValue() const { return value.get(); }
	int getLine() const override { return line; }

private:
	const std::string name;
	std::unique_ptr<Node> value;
	int line;
};

class NodeBlock : public Node
{
public:
	explicit NodeBlock(int line) : line(line) {}

	void addStatement(std::unique_ptr<Node> stmt)
	{
		statements.push_back(std::move(stmt));
	}

	const std::vector<std::unique_ptr<Node>> &getStatements() const { return statements; }
	int getLine() const override { return line; }

private:
	std::vector<std::unique_ptr<Node>> statements;
	int line;
};

class Parser
{
public:
	explicit Parser(std::vector<Token> &tokens) : tokens(tokens), position(0) {}

	std::unique_ptr<Node> parse();

private:
	std::unique_ptr<Node> parseExpression();
	std::unique_ptr<Node> parseComparison();
	std::unique_ptr<Node> parseAdditive();
	std::unique_ptr<Node> parsePrimary();
	std::unique_ptr<Node> parseUnary();
	std::unique_ptr<Node> parseTerm();
	std::unique_ptr<Node> parseFactor();

	std::unique_ptr<Node> parseStatement();
	std::unique_ptr<NodeBlock> parseBlock();
	std::unique_ptr<Node> parseReturn();
	std::unique_ptr<Node> parseVariableDeclaration();
	std::unique_ptr<Node> parseFuncDeclaration();
	std::unique_ptr<Node> parseAssignment();

	bool matchMultipleTokens(const std::vector<Token::Kind> &kinds);
	bool matchSingleToken(const Token::Kind kind);
	bool isAtEnd() const;

	Token peek() const;
	Token previous() const;
	Token advance();

	const std::vector<Token> tokens;
	size_t position;
};
