#include "../include/codegen.hpp"

void SymbolTable::addVariable(const std::string &name, llvm::Value *value, llvm::Type *type)
{
    scopes.back()[name] = {value, type};
}

const SymbolTable::Symbol *SymbolTable::lookupVariable(const std::string &name) const
{
    for (auto scopeIt = scopes.rbegin(); scopeIt != scopes.rend(); scopeIt++)
    {
        auto it = scopeIt->find(name);
        if (it != scopeIt->end())
        {
            return &it->second;
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

llvm::Value *CodeGenerator::generateExpression(const Node *node, llvm::Type *expectedType)
{
    if (auto number = dynamic_cast<const NodeNumber *>(node))
    {
        if (!expectedType)
        {
            fprintf(stderr, "Expected type\n");
            return nullptr;
        }

        unsigned bitWidth = expectedType->getIntegerBitWidth();
        return llvm::ConstantInt::get(context, llvm::APInt(bitWidth, number->getValue(), true));
    }
    else if (auto binary = dynamic_cast<const NodeBinaryOp *>(node))
    {
        llvm::Value *left = generateExpression(binary->getLeft(), expectedType);
        llvm::Value *right = generateExpression(binary->getRight(), expectedType);

        if (!left || !right)
            return nullptr;

        left = castValue(left, expectedType);
        right = castValue(right, expectedType);

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
        const SymbolTable::Symbol *symbol = symbolTable.lookupVariable(identifier->getName());

        if (!symbol)
        {
            fprintf(stderr, "Undefined variable: %s\n", identifier->getName().c_str());
            return nullptr;
        }

        llvm::Value *value = builder.CreateLoad(symbol->type, symbol->value, identifier->getName().c_str());

        if (expectedType)
            value = castValue(value, expectedType);

        return value;
    }

    return nullptr;
}

llvm::Value *CodeGenerator::generateVarDeclaration(const NodeVarDeclaration *node, llvm::Function *function)
{
    llvm::Type *llvmType;

    if (node->getType() == "i8")
    {
        llvmType = llvm::Type::getInt8Ty(context);
    }
    else if (node->getType() == "i16")
    {
        llvmType = llvm::Type::getInt16Ty(context);
    }
    else if (node->getType() == "i32")
    {
        llvmType = llvm::Type::getInt32Ty(context);
    }
    else if (node->getType() == "i64")
    {
        llvmType = llvm::Type::getInt64Ty(context);
    }
    else
    {
        fprintf(stderr, "Unknown type: %s", node->getType());
    }

    llvm::AllocaInst *alloca = createEntryBlockAlloca(function, node->getName(), llvmType);

    symbolTable.addVariable(node->getName(), alloca, llvmType);

    llvm::Value *initializer = generateExpression(node->getInitializer(), llvmType);

    if (!initializer)
        return nullptr;

    return builder.CreateStore(initializer, alloca);
}

llvm::Value *CodeGenerator::castValue(llvm::Value *value, llvm::Type *expectedType)
{
    if (value->getType() == expectedType)
        return value;

    if (value->getType()->isIntegerTy() && expectedType->isIntegerTy())
    {
        unsigned srcBits = value->getType()->getIntegerBitWidth();
        unsigned dstBits = expectedType->getIntegerBitWidth();

        if (srcBits < dstBits)
            return builder.CreateSExt(value, expectedType, "sext");
        else if (srcBits > dstBits)
            return builder.CreateTrunc(value, expectedType, "trunc");
    }

    return value;
}

llvm::AllocaInst *CodeGenerator::createEntryBlockAlloca(llvm::Function *function, const std::string &varName, llvm::Type *varType)
{
    llvm::IRBuilder<> tmpB(&function->getEntryBlock(), function->getEntryBlock().begin());
    return tmpB.CreateAlloca(varType, nullptr, varName);
}