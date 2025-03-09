#pragma once

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include <llvm/Support/raw_ostream.h>
#include "../include/parser.hpp"
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

class SymbolTable
{
public:
    SymbolTable() { scopes.emplace_back(); }

    struct Symbol
    {
        llvm::Value *value;
        llvm::Type *type;
        std::string baseType;
    };

    void addVariable(const std::string &name, llvm::Value *value, llvm::Type *type, const std::string &baseType);
    const Symbol *lookupVariable(const std::string &name) const;
    void enterScope();
    void exitScope();

private:
    std::vector<std::unordered_map<std::string, Symbol>> scopes;
};

class CodeGenerator
{
public:
    CodeGenerator(const std::string &moduleName)
        : context(), module(std::make_unique<llvm::Module>(moduleName, context)), builder(context), currentFunction(nullptr), hasReturn(false) {}

    void generate(const Node *root);
    void generateRuntime();
    llvm::Module *getModule() const { return module.get(); }

private:
    llvm::Type *getLLVMType(const std::string &typeName);
    llvm::Type *getBaseLLVMType(const std::string &baseTypeStr);
    std::string parseArraySizes(const std::string &typeName, std::vector<uint64_t> &arraySizes);
    llvm::Type *applyArraySizes(llvm::Type *baseType, const std::vector<uint64_t> &arraySizes);

    llvm::Value *generateExpression(const Node *node, llvm::Type *expectedType = nullptr);
    llvm::Value *handleUnaryOp(const class NodeUnaryOp *node, llvm::Type *expectedType);
    llvm::Value *handleCast(const class NodeCast *node);
    llvm::Value *handleString(const class NodeString *node);
    llvm::Value *handleAssignment(const class NodeAssignment *node, llvm::Type *expectedType);
    llvm::Value *handlePointerAssignment(const class NodePointerAssignment *node);
    llvm::Value *handleArrayAccess(const class NodeArrayAccess *node);
    llvm::Value *handleArrayAssignment(const class NodeArrayAssignment *node);
    llvm::Value *handleIdentifier(const class NodeIdentifier *node);
    llvm::Value *handleNumber(const class NodeNumber *node, llvm::Type *expectedType);
    llvm::Value *handleBinaryOp(const class NodeBinaryOp *node, llvm::Type *expectedType);
    llvm::Value *handleFunctionCall(const class NodeFunctionCall *node, llvm::Type *expectedType);

    void generateFuncDeclaration(const class NodeFunctionDeclaration *node);
    void generateExternDeclaration(const class NodeExternDeclaration *node);
    void generateStatement(const Node *stmt);
    llvm::Value *generateVarDeclaration(const class NodeVariableDeclaration *node);
    void generateWhileStatement(const NodeWhile* node);
    void generateReturn(const class NodeReturn *node);
    llvm::Value *castValue(llvm::Value *value, llvm::Type *expectedType);
    llvm::AllocaInst *createEntryBlockAlloca(llvm::Function *function, const std::string &varName, llvm::Type *varType);

    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> module;
    llvm::IRBuilder<> builder;
    llvm::Function *currentFunction;
    bool hasReturn;

    std::unordered_map<std::string, llvm::StructType *> structTypes;
    std::unordered_map<std::string, std::vector<std::string>> structFieldNames;

    SymbolTable symbolTable;
};
