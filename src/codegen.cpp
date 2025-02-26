#include "../include/codegen.hpp"

void SymbolTable::addVariable(const std::string &name, llvm::Value *value)
{
    scopes.back()[name] = value;
}

llvm::Value *SymbolTable::lookupVariable(const std::string &name) const
{
    for (auto scopeIt = scopes.rbegin(); scopeIt != scopes.rend(); scopeIt++)
    {
        auto it = scopeIt->find(name);

        if (it != scopeIt->end())
        {
            return it->second;
        }
    }

    return nullptr;
}

void SymbolTable::enterScope()
{
    scopes.emplace_back();
}

void SymbolTable::exitScope()
{
    if (scopes.size() > 1)
        scopes.pop_back();
    else
        fprintf(stderr, "Attempt to exit the global scope is not allowed");
}

void CodeGenerator::generate(const Node *root)
{
    llvm::FunctionType *funcType = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), false);
    llvm::Function *mainFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", module.get());
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entry", mainFunc);

    builder.SetInsertPoint(entry);

    if (const NodeBlock *block = dynamic_cast<const NodeBlock *>(root))
    {
        for (const auto &stmt : block->getStatements())
        {
            if (auto varDecl = dynamic_cast<const NodeVarDeclaration *>(stmt.get()))
            {
                generateVarDeclaration(varDecl, mainFunc);
            }
        }
    }

    builder.CreateRet(llvm::ConstantInt::get(context, llvm::APInt(32, 0)));
}

llvm::Value *CodeGenerator::generateExpression(const Node *node)
{
    if (auto number = dynamic_cast<const NodeNumber *>(node))
    {
        return llvm::ConstantInt::get(context, llvm::APInt(32, number->getValue(), true));
    }
    else if (auto binary = dynamic_cast<const NodeBinaryOp *>(node))
    {
        llvm::Value *left = generateExpression(binary->getLeft());
        llvm::Value *right = generateExpression(binary->getRight());
        if (!left || !right)
            return nullptr;

        switch (binary->getOp())
        {
        case Token::Kind::TOKEN_PLUS:
            return builder.CreateAdd(left, right, "addtmp");
        case Token::Kind::TOKEN_MINUS:
            return builder.CreateSub(left, right, "subtmp");
        case Token::Kind::TOKEN_STAR:
            return builder.CreateMul(left, right, "multmp");
        case Token::Kind::TOKEN_SLASH:
            return builder.CreateSDiv(left, right, "divtmp");
        default:
            return nullptr;
        }
    }
    else if (auto identifier = dynamic_cast<const NodeIdentifier *>(node))
    {
        llvm::Value *varPtr = symbolTable.lookupVariable(identifier->getName());

        if (!varPtr)
            fprintf(stderr, "Undefined variable: %s", identifier->getName());

        return builder.CreateLoad(llvm::Type::getInt32Ty(context), varPtr, identifier->getName().c_str());
    }

    return nullptr;
}

llvm::Value *CodeGenerator::generateVarDeclaration(const NodeVarDeclaration *node, llvm::Function *function)
{
    llvm::Type *llvmType = llvm::Type::getInt32Ty(context);
    llvm::AllocaInst *alloca = createEntryBlockAlloca(function, node->getName(), llvmType);

    symbolTable.addVariable(node->getName(), alloca);

    llvm::Value *initializer = generateExpression(node->getInitializer());

    if (!initializer)
        return nullptr;

    return builder.CreateStore(initializer, alloca);
}

llvm::AllocaInst *CodeGenerator::createEntryBlockAlloca(llvm::Function *function, const std::string &varName, llvm::Type *varType)
{
    llvm::IRBuilder<> tmpB(&function->getEntryBlock(), function->getEntryBlock().begin());
    return tmpB.CreateAlloca(varType, nullptr, varName);
}