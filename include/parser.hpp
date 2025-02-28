#pragma once

#include <memory>
#include "lexer.hpp"

class Node
{
public:
	virtual ~Node() = default;
};

class NodeNumber : public Node
{
public:
	explicit NodeNumber(int value) : value(value) {}

	int getValue() const { return value; }

private:
	int value;
};

class NodeIdentifier : public Node
{
public:
	explicit NodeIdentifier(const std::string &name) : name(name) {}

	const std::string &getName() const { return name; }

private:
	std::string name;
};

class NodeBinaryOp : public Node
{
public:
	NodeBinaryOp(Token::Kind op, std::unique_ptr<Node> left, std::unique_ptr<Node> right) : op(op), left(std::move(left)), right(std::move(right)) {}

	Token::Kind getOp() const { return op; }
	const Node *getLeft() const { return left.get(); }
	const Node *getRight() const { return right.get(); }

private:
	Token::Kind op;
	std::unique_ptr<Node> left;
	std::unique_ptr<Node> right;
};

class NodeVarDeclaration : public Node
{
public:
	NodeVarDeclaration(const std::string &name, std::string &type, std::unique_ptr<Node> initializer) : name(name), type(type), initializer(std::move(initializer)) {}

	const std::string &getName() const { return name; }
	const std::string &getType() const { return type; }
	const Node *getInitializer() const { return initializer.get(); }

private:
	std::string name;
	std::string type;
	std::unique_ptr<Node> initializer;
};

class NodeFuncDeclaration : public Node
{
public:
	NodeFuncDeclaration(const std::string &name, const std::vector<std::pair<std::string, std::string>> &args, std::unique_ptr<Node> body, const std::string &returnType) : name(name), args(args), body(std::move(body)), returnType(returnType) {}

	const std::string &getName() const { return name; }
	const std::vector<std::pair<std::string, std::string>> &getArgs() const { return args; }
	const std::unique_ptr<Node> &getBody() const { return body; }
	const std::string &getReturnType() const { return returnType; }

private:
	std::string name;
	std::vector<std::pair<std::string, std::string>> args;
	std::unique_ptr<Node> body;
	std::string returnType;
};

class NodeAssignment : public Node
{
public:
	NodeAssignment(const std::string &name, std::unique_ptr<Node> value) : name(name), value(std::move(value)) {}

	const std::string &getName() const { return name; }
	const Node *getValue() const { return value.get(); }

private:
	const std::string name;
	std::unique_ptr<Node> value;
};

class NodeBlock : public Node
{
public:
	void addStatement(std::unique_ptr<Node> stmt) { statements.push_back(std::move(stmt)); }

	const std::vector<std::unique_ptr<Node>> &getStatements() const { return statements; }

private:
	std::vector<std::unique_ptr<Node>> statements;
};

class Parser
{
public:
	explicit Parser(std::vector<Token> &tokens) : tokens(tokens), position(0) {}

	std::unique_ptr<Node> parse();

private:
	std::unique_ptr<Node> parseExpression();
	std::unique_ptr<Node> parseTerm();
	std::unique_ptr<Node> parseFactor();

	std::unique_ptr<Node> parseStatement();
	std::unique_ptr<NodeBlock> parseBlock();
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
