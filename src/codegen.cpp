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

llvm::Type *CodeGenerator::getLLVMType(const std::string &typeName)
{
    if (typeName == "i8")
    {
        return llvm::Type::getInt8Ty(context);
    }
    else if (typeName == "i16")
    {
        return llvm::Type::getInt16Ty(context);
    }
    else if (typeName == "i32")
    {
        return llvm::Type::getInt32Ty(context);
    }
    else if (typeName == "i64")
    {
        return llvm::Type::getInt64Ty(context);
    }
    else if (typeName == "void")
    {
        return llvm::Type::getVoidTy(context);
    }
    else
    {
        return nullptr;
    }
}

void CodeGenerator::generate(const Node *root)
{
    if (const NodeBlock *block = dynamic_cast<const NodeBlock *>(root))
    {
        for (const auto &stmt : block->getStatements())
        {
            if (auto varDecl = dynamic_cast<const NodeVarDeclaration *>(stmt.get()))
            {
                llvm::Type *llvmType = getLLVMType(varDecl->getType());
                if (!llvmType)
                {
                    fprintf(stderr, "Unknown type: %s\n", varDecl->getType().c_str());
                    continue;
                }

                llvm::GlobalVariable *globalVar = new llvm::GlobalVariable(
                    *module,
                    llvmType,
                    false,
                    llvm::GlobalValue::ExternalLinkage,
                    nullptr,
                    varDecl->getName());

                symbolTable.addVariable(varDecl->getName(), globalVar, llvmType);

                if (varDecl->getInitializer())
                {
                    llvm::Value *initializer = generateExpression(varDecl->getInitializer(), llvmType);

                    if (initializer)
                    {
                        globalVar->setInitializer(llvm::cast<llvm::Constant>(initializer));
                    }
                }
            }
            else if (auto funcDecl = dynamic_cast<const NodeFuncDeclaration *>(stmt.get()))
            {
                generateFuncDeclaration(funcDecl);
            }
        }
    }
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
        case Token::Kind::TOKEN_EQUAL_EQUAL:
        {
            llvm::Value *cmp = builder.CreateICmpEQ(left, right, "eqtmp");
            return builder.CreateZExt(cmp, expectedType, "zexttmp");
        }
        case Token::Kind::TOKEN_BANG_EQUAL:
        {
            llvm::Value *cmp = builder.CreateICmpNE(left, right, "netmp");
            return builder.CreateZExt(cmp, expectedType, "zexttmp");
        }
        case Token::Kind::TOKEN_LESS:
        {
            llvm::Value *cmp = builder.CreateICmpSLT(left, right, "slttmp");
            return builder.CreateZExt(cmp, expectedType, "zexttmp");
        }
        case Token::Kind::TOKEN_LESS_EQUAL:
        {
            llvm::Value *cmp = builder.CreateICmpSLE(left, right, "sletmp");
            return builder.CreateZExt(cmp, expectedType, "zexttmp");
        }
        case Token::Kind::TOKEN_GREATER:
        {
            llvm::Value *cmp = builder.CreateICmpSGT(left, right, "sgttmp");
            return builder.CreateZExt(cmp, expectedType, "zexttmp");
        }
        case Token::Kind::TOKEN_GREATER_EQUAL:
        {
            llvm::Value *cmp = builder.CreateICmpSGE(left, right, "sgetmp");
            return builder.CreateZExt(cmp, expectedType, "zexttmp");
        }
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

void CodeGenerator::generateReturn(const NodeReturn *node, llvm::Type *expectedType)
{
    if (node->getExpression())
    {
        llvm::Value *returnValue = generateExpression(node->getExpression(), expectedType);

        if (!returnValue)
        {
            fprintf(stderr, "Error: Invalid return expression.\n");
            return;
        }

        builder.CreateRet(returnValue);
    }
    else
    {
        if (expectedType->isVoidTy())
        {
            builder.CreateRetVoid();
        }
        else
        {
            fprintf(stderr, "Non-void function must return a value.\n");
        }
    }
}

llvm::Value *CodeGenerator::generateVarDeclaration(const NodeVarDeclaration *node, llvm::Function *function)
{
    llvm::Type *llvmType = getLLVMType(node->getType());

    if (!llvmType)
    {
        fprintf(stderr, "Unknown type: %s\n", node->getType().c_str());
        return nullptr;
    }

    llvm::AllocaInst *alloca = createEntryBlockAlloca(function, node->getName(), llvmType);

    symbolTable.addVariable(node->getName(), alloca, llvmType);

    llvm::Value *initializer = generateExpression(node->getInitializer(), llvmType);

    if (!initializer)
        return nullptr;

    return builder.CreateStore(initializer, alloca);
}

void CodeGenerator::generateFuncDeclaration(const NodeFuncDeclaration *node)
{
    std::vector<llvm::Type *> argTypes;

    for (const auto &arg : node->getArgs())
    {
        llvm::Type *argType = getLLVMType(arg.second);

        if (!argType)
        {
            fprintf(stderr, "Unknown argument type: %s\n", arg.second.c_str());
            return;
        }

        argTypes.push_back(argType);
    }

    llvm::Type *returnType = getLLVMType(node->getReturnType());

    if (!returnType)
    {
        fprintf(stderr, "Unknown return type: %s\n", node->getReturnType().c_str());
        return;
    }

    llvm::FunctionType *funcType = llvm::FunctionType::get(returnType, argTypes, false);
    llvm::Function *function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, node->getName(), module.get());
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entry", function);

    builder.SetInsertPoint(entry);
    symbolTable.enterScope();

    unsigned idx = 0;

    for (auto &arg : function->args())
    {
        std::string argName = node->getArgs()[idx].first;
        arg.setName(argName);

        llvm::AllocaInst *alloca = createEntryBlockAlloca(function, argName, arg.getType());
        builder.CreateStore(&arg, alloca);

        symbolTable.addVariable(argName, alloca, arg.getType());
        idx++;
    }

    bool hasReturn = false;
    if (const NodeBlock *body = dynamic_cast<const NodeBlock *>(node->getBody().get()))
    {
        for (const auto &stmt : body->getStatements())
        {
            if (auto varDecl = dynamic_cast<const NodeVarDeclaration *>(stmt.get()))
            {
                generateVarDeclaration(varDecl, function);
            }
            else if (auto returnStmt = dynamic_cast<const NodeReturn *>(stmt.get()))
            {
                generateReturn(returnStmt, returnType);
                hasReturn = true;
            }
        }
    }

    if (!hasReturn)
    {
        if (returnType->isVoidTy())
        {
            builder.CreateRetVoid();
        }
        else
        {
            fprintf(stderr, "Function '%s' with return type '%s' must have a return statement.\n", node->getName().c_str(), node->getReturnType().c_str());

            return;
        }
    }

    symbolTable.exitScope();
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