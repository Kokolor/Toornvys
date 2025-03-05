#pragma once

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include <llvm/Support/raw_ostream.h>
#include "../include/parser.hpp"

class SymbolTable
{
public:
    SymbolTable() { scopes.emplace_back(); }

    struct Symbol
    {
        llvm::Value *value;
        llvm::Type *type;
    };

    void addVariable(const std::string &name, llvm::Value *value, llvm::Type *type);
    const Symbol *lookupVariable(const std::string &name) const;
    void enterScope();
    void exitScope();

private:
    std::vector<std::unordered_map<std::string, Symbol>> scopes;
};

class CodeGenerator
{
public:
    CodeGenerator(const std::string &moduleName) : context(), module(std::make_unique<llvm::Module>(moduleName, context)), builder(context) {}

    void generate(const Node *root);
    llvm::Module *getModule() const { return module.get(); }

private:
    llvm::Type *getLLVMType(const std::string &typeName);

    llvm::Value *generateExpression(const Node *node, llvm::Type *expectedType = nullptr);
    void generateReturn(const NodeReturn *node, llvm::Type *expectedType);
    llvm::Value *generateVarDeclaration(const NodeVarDeclaration *node, llvm::Function *function);
    void generateFuncDeclaration(const NodeFuncDeclaration *node);
    llvm::Value *castValue(llvm::Value *value, llvm::Type *expectedType);
    llvm::AllocaInst *createEntryBlockAlloca(llvm::Function *function, const std::string &varName, llvm::Type *varType);

    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> module;
    llvm::IRBuilder<> builder;

    SymbolTable symbolTable;
};