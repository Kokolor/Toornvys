#include <iostream>
#include <fstream>
#include <sstream>
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
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
        std::cout << indentation << "VarDeclaration(" << varDecl->getName() << ": " << varDecl->getType() << ")\n";
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
    else if (const NodeFuncDeclaration *funcDecl = dynamic_cast<const NodeFuncDeclaration *>(node))
    {
        std::cout << indentation << "FuncDeclaration(" << funcDecl->getName() << ")\n";
        std::cout << indentation << "  ReturnType: " << funcDecl->getReturnType() << "\n";

        std::cout << indentation << "  Args:\n";
        for (const auto &arg : funcDecl->getArgs())
        {
            std::cout << indentation << "    " << arg.first << ": " << arg.second << "\n";
        }

        printAST(funcDecl->getBody().get(), indent + 2);
    }
    else
    {
        std::cout << indentation << "Unknown Node\n";
    }
}

std::string readSourceFile(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Neuil while opening the file '" << filename << "'" << std::endl;
        exit(1);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <file>" << std::endl;
        return 1;
    }

    std::string sourceCode = readSourceFile(argv[1]);

    Lexer lexer(sourceCode);
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

    llvm::PassBuilder passBuilder;
    llvm::LoopAnalysisManager LAM;
    llvm::FunctionAnalysisManager FAM;
    llvm::CGSCCAnalysisManager CGAM;
    llvm::ModuleAnalysisManager MAM;

    passBuilder.registerModuleAnalyses(MAM);
    passBuilder.registerCGSCCAnalyses(CGAM);
    passBuilder.registerFunctionAnalyses(FAM);
    passBuilder.registerLoopAnalyses(LAM);
    passBuilder.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    llvm::ModulePassManager modulePM = passBuilder.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3);

    modulePM.run(*codegen.getModule(), MAM);

    codegen.getModule()->print(llvm::outs(), nullptr);

    return 0;
}