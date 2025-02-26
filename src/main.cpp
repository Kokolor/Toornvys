#include <iostream>
#include "../include/lexer.hpp"
#include "../include/parser.hpp"
#include "../include/codegen.hpp"

std::string tokenKindToString(Token::Kind kind)
{
    switch (kind)
    {
    case Token::Kind::TOKEN_PLUS:
        return "+";
    case Token::Kind::TOKEN_MINUS:
        return "-";
    case Token::Kind::TOKEN_STAR:
        return "*";
    case Token::Kind::TOKEN_SLASH:
        return "/";
    case Token::Kind::TOKEN_EQUAL:
        return "=";
    case Token::Kind::TOKEN_SEMI:
        return ";";
    case Token::Kind::TOKEN_LET:
        return "let";
    case Token::Kind::TOKEN_FN:
        return "fn";
    default:
        return "?";
    }
}

void printAST(const Node *node, int indent = 0)
{
    std::string indentation(indent, ' ');

    if (const NodeNumber *num = dynamic_cast<const NodeNumber *>(node))
    {
        std::cout << indentation << "Number(" << num->getValue() << ")\n";
    }
    else if (const NodeIdentifier *id = dynamic_cast<const NodeIdentifier *>(node))
    {
        std::cout << indentation << "Identifier(" << id->getName() << ")\n";
    }
    else if (const NodeBinaryOp *op = dynamic_cast<const NodeBinaryOp *>(node))
    {
        std::cout << indentation << "BinaryOp("
                  << tokenKindToString(op->getOp()) << ")\n";
        printAST(op->getLeft(), indent + 2);
        printAST(op->getRight(), indent + 2);
    }
    else if (const NodeVarDeclaration *varDecl = dynamic_cast<const NodeVarDeclaration *>(node))
    {
        std::cout << indentation << "VarDeclaration(" << varDecl->getName() << ")\n";
        printAST(varDecl->getInitializer(), indent + 2);
    }
    else if (const NodeAssignment *assign = dynamic_cast<const NodeAssignment *>(node))
    {
        std::cout << indentation << "Assignment(" << assign->getName() << ")\n";
        printAST(assign->getValue(), indent + 2);
    }
    else if (const NodeBlock *block = dynamic_cast<const NodeBlock *>(node))
    {
        std::cout << indentation << "Block\n";
        for (const auto &stmt : block->getStatements())
        {
            printAST(stmt.get(), indent + 2);
        }
    }
    else
    {
        std::cout << indentation << "Unknown Node\n";
    }
}

int main()
{
    Lexer lexer("let hello = 4 + 2;\nlet test = hello * 2 + 4;");
    std::vector<Token> tokens = lexer.tokenize();

    for (const Token &token : tokens)
    {
        std::cout << "Token: " << token.getValue() << " of kind "
                  << static_cast<int>(token.getKind()) << std::endl;
    }

    Parser parser(tokens);
    std::unique_ptr<Node> ast = parser.parse();

    std::cout << "\nAST Structure:\n";
    printAST(ast.get());

    CodeGenerator codegen("main_module");
    codegen.generate(ast.get());

    codegen.getModule()->print(llvm::outs(), nullptr);

    return 0;
}
