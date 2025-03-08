#include "../include/error.hpp"
#include "../include/codegen.hpp"

void SymbolTable::addVariable(const std::string &name, llvm::Value *value, llvm::Type *type, const std::string &baseType)
{
    scopes.back()[name] = {value, type, baseType};
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
        ERROR(0, "Attempt to exit the global scope is not allowed");
}

llvm::Type *CodeGenerator::getLLVMType(const std::string &typeName)
{
    if (!typeName.empty() && typeName.back() == '*')
    {
        std::string baseTypeName = typeName.substr(0, typeName.size() - 1);
        llvm::Type *baseType = getLLVMType(baseTypeName);

        if (!baseType)
            ERROR(0, "Unknown base type: %s\n", baseTypeName.c_str());

        return llvm::PointerType::getUnqual(baseType);
    }

    if (typeName == "string")
    {
        return llvm::PointerType::get(llvm::Type::getInt8Ty(context), 0);
    }

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
                    ERROR(0, "Unknown type: %s\n", varDecl->getType().c_str());

                bool isConstant = (varDecl->getInitializer() != nullptr);

                llvm::GlobalVariable *globalVar = new llvm::GlobalVariable(
                    *module,
                    llvmType,
                    isConstant,
                    llvm::GlobalValue::ExternalLinkage,
                    nullptr,
                    varDecl->getName());

                symbolTable.addVariable(varDecl->getName(), globalVar, llvmType, varDecl->getType());

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
            else if (auto externDecl = dynamic_cast<const NodeExternDeclaration *>(stmt.get()))
            {
                generateExternDeclaration(externDecl);
            }
        }
    }
}

void CodeGenerator::generateRuntime()
{
    llvm::FunctionType *printIntType = llvm::FunctionType::get(llvm::Type::getVoidTy(context), {llvm::Type::getInt32Ty(context)}, false);
    llvm::Function *printIntFunc = llvm::Function::Create(printIntType, llvm::Function::ExternalLinkage, "printInt", module.get());
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entry", printIntFunc);
    builder.SetInsertPoint(entry);

    llvm::FunctionType *printfType = llvm::FunctionType::get(llvm::IntegerType::getInt32Ty(context), llvm::PointerType::get(llvm::Type::getInt8Ty(context), 0), true);
    llvm::Function *printfFunc = llvm::Function::Create(printfType, llvm::Function::ExternalLinkage, "printf", module.get());

    llvm::Value *formatStr = builder.CreateGlobalStringPtr("%d\n");

    llvm::Argument *arg = &*printIntFunc->arg_begin();

    std::vector<llvm::Value *> args = {formatStr, arg};
    builder.CreateCall(printfFunc, args);
    builder.CreateRetVoid();
}

llvm::Value *CodeGenerator::generateExpression(const Node *node, llvm::Type *expectedType)
{
    if (auto unary = dynamic_cast<const NodeUnaryOp *>(node))
    {
        if (unary->getOp() == Token::Kind::TOKEN_STAR)
        {
            llvm::Value *ptr = generateExpression(unary->getOperand(), nullptr);

            if (!ptr)
                ERROR(node->getLine(), "Invalid pointer expression for dereference.\n");

            if (!expectedType)
                ERROR(node->getLine(), "Expected type must be provided for dereference with opaque pointers.\n");

            return builder.CreateLoad(expectedType, ptr, "deref");
        }
        else if (unary->getOp() == Token::Kind::TOKEN_AMPERSAND)
        {
            if (auto id = dynamic_cast<const NodeIdentifier *>(unary->getOperand()))
            {
                const SymbolTable::Symbol *sym = symbolTable.lookupVariable(id->getName());

                if (!sym)
                    ERROR(node->getLine(), "Undefined variable: %s\n", id->getName().c_str());

                return sym->value;
            }
            else
                ERROR(node->getLine(), "L'opérateur '&' ne peut s'appliquer qu'à un identifiant. T UN NEUIL FRR, PROUT PROUT PROUT\n");
        }
    }

    if (auto cast = dynamic_cast<const NodeCast *>(node))
    {
        llvm::Value *exprVal = generateExpression(cast->getExpression(), nullptr);
        llvm::Type *targetType = getLLVMType(cast->getTargetType());

        return castValue(exprVal, targetType);
    }

    if (auto strNode = dynamic_cast<const NodeString *>(node))
    {
        llvm::Value *str = builder.CreateGlobalStringPtr(strNode->getValue());
        return str;
    }

    if (auto assign = dynamic_cast<const NodeAssignment *>(node))
    {
        const SymbolTable::Symbol *sym = symbolTable.lookupVariable(assign->getName());

        if (!sym)
            ERROR(assign->getLine(), "Undefined variable: %s", assign->getName().c_str());

        llvm::Value *value = generateExpression(assign->getValue(), sym->type);

        if (!value)
            return nullptr;

        builder.CreateStore(value, sym->value);

        return value;
    }
    else if (auto ptrAssign = dynamic_cast<const NodePointerAssignment *>(node))
    {
        llvm::Value *ptr = generateExpression(ptrAssign->getPointer(), nullptr);

        if (auto ptrNode = dynamic_cast<const NodeIdentifier *>(ptrAssign->getPointer()))
        {
            const SymbolTable::Symbol *sym = symbolTable.lookupVariable(ptrNode->getName());
            if (sym && !sym->baseType.empty() && sym->baseType.back() == '*')
            {
                std::string pointeeType = sym->baseType.substr(0, sym->baseType.size() - 1);
                llvm::Type *expectedType = getLLVMType(pointeeType);

                llvm::Value *value = generateExpression(ptrAssign->getValue(), expectedType);
                if (!value)
                    return nullptr;

                builder.CreateStore(value, ptr);
                return value;
            }
        }

        ERROR(ptrAssign->getLine(), "Invalid Pointer Assignment");
    }
    else if (auto id = dynamic_cast<const NodeIdentifier *>(node))
    {
        const SymbolTable::Symbol *sym = symbolTable.lookupVariable(id->getName());

        if (!sym)
            ERROR(id->getLine(), "Undefined variable: %s", id->getName().c_str());

        return builder.CreateLoad(sym->type, sym->value, id->getName() + ".val");
    }
    if (auto number = dynamic_cast<const NodeNumber *>(node))
    {
        if (!expectedType)
            ERROR(node->getLine(), "Expected type\n");

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
    else if (auto call = dynamic_cast<const NodeFunctionCall *>(node))
    {
        llvm::Function *function = module->getFunction(call->getName());

        if (!function)
            ERROR(call->getLine(), "Undefined function '%s'", call->getName().c_str());

        if (function->arg_size() != call->getArgs().size())
            ERROR(call->getLine(), "Function '%s' expects %d arguments but got %d", call->getName().c_str(), function->arg_size(), call->getArgs().size());

        std::vector<llvm::Value *> args;
        size_t i = 0;

        for (auto &arg : call->getArgs())
        {
            llvm::Type *expectedArgType = function->getArg(i)->getType();
            llvm::Value *argValue = generateExpression(arg.get(), expectedArgType);

            if (!argValue)
                return nullptr;

            args.push_back(argValue);
            i++;
        }

        return builder.CreateCall(function, args, "calltmp");
    }

    return nullptr;
}

void CodeGenerator::generateFuncDeclaration(const NodeFuncDeclaration *node)
{
    std::vector<llvm::Type *> argTypes;

    for (const auto &arg : node->getArgs())
    {
        llvm::Type *argType = getLLVMType(arg.second);
        if (!argType)
            ERROR(node->getLine(), "Unknown argument type: %s\n", arg.second.c_str());
        argTypes.push_back(argType);
    }

    llvm::Type *returnType = getLLVMType(node->getReturnType());

    if (!returnType)
        ERROR(node->getLine(), "Unknown return type: %s\n", node->getReturnType().c_str());

    llvm::FunctionType *funcType = llvm::FunctionType::get(returnType, argTypes, false);
    llvm::Function *function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, node->getName(), module.get());
    currentFunction = function;
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
        symbolTable.addVariable(argName, alloca, arg.getType(), node->getArgs()[idx].second);
        idx++;
    }

    hasReturn = false;

    if (const NodeBlock *body = dynamic_cast<const NodeBlock *>(node->getBody().get()))
    {
        for (const auto &stmt : body->getStatements())
        {
            generateStatement(stmt.get());
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
            ERROR(node->getLine(), "Function '%s' with return type '%s' must have a return statement.\n",
                  node->getName().c_str(), node->getReturnType().c_str());
        }
    }

    symbolTable.exitScope();
    currentFunction = nullptr;
}

void CodeGenerator::generateExternDeclaration(const NodeExternDeclaration *node)
{
    std::vector<llvm::Type *> argTypes;

    for (const auto &arg : node->getArgs())
    {
        llvm::Type *argType = getLLVMType(arg.second);

        if (!argType)
            ERROR(node->getLine(), "Unknown argument type: %s\n", arg.second.c_str());

        argTypes.push_back(argType);
    }

    llvm::Type *returnType = getLLVMType(node->getReturnType());

    if (!returnType)
        ERROR(node->getLine(), "Unknown return type: %s\n", node->getReturnType().c_str());

    llvm::FunctionType *funcType = llvm::FunctionType::get(returnType, argTypes, false);
    llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, node->getName(), module.get());
}

void CodeGenerator::generateStatement(const Node *stmt)
{
    if (auto varDecl = dynamic_cast<const NodeVarDeclaration *>(stmt))
    {
        generateVarDeclaration(varDecl);
    }
    else if (auto returnStmt = dynamic_cast<const NodeReturn *>(stmt))
    {
        generateReturn(returnStmt);
    }
    else
    {
        generateExpression(stmt, nullptr);
    }
}

llvm::Value *CodeGenerator::generateVarDeclaration(const NodeVarDeclaration *node)
{
    llvm::Type *llvmType = getLLVMType(node->getType());

    if (!llvmType)
        ERROR(node->getLine(), "Unknown type: %s\n", node->getType().c_str());

    llvm::AllocaInst *alloca = createEntryBlockAlloca(currentFunction, node->getName(), llvmType);
    symbolTable.addVariable(node->getName(), alloca, llvmType, node->getType());

    if (node->getInitializer())
    {
        llvm::Value *initializer = generateExpression(node->getInitializer(), llvmType);
        if (initializer)
            builder.CreateStore(initializer, alloca);
    }

    return alloca;
}

void CodeGenerator::generateReturn(const NodeReturn *node)
{
    hasReturn = true;
    llvm::Type *expectedType = currentFunction->getReturnType();

    if (node->getExpression())
    {
        llvm::Value *returnValue = generateExpression(node->getExpression(), expectedType);
        if (!returnValue)
            ERROR(node->getLine(), "Invalid return expression");

        if (returnValue->getType() != expectedType)
        {
            std::string expectedStr, actualStr;
            llvm::raw_string_ostream rsoExpected(expectedStr);
            llvm::raw_string_ostream rsoActual(actualStr);
            expectedType->print(rsoExpected);
            returnValue->getType()->print(rsoActual);
            ERROR(node->getLine(), "Type mismatch: Function returns '%s' but got '%s'",
                  rsoExpected.str().c_str(), rsoActual.str().c_str());
        }

        builder.CreateRet(returnValue);
    }
    else
    {
        if (expectedType->isVoidTy())
            builder.CreateRetVoid();
        else
            ERROR(node->getLine(), "Non-void function must return a value");
    }
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