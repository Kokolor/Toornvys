#include <iostream>
#include <fstream>
#include <sstream>
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/FileSystem.h"
#include "../include/lexer.hpp"
#include "../include/parser.hpp"
#include "../include/codegen.hpp"

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
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <input-file> <output-file>" << std::endl;
        return 1;
    }

    std::string inputFilename = argv[1];
    std::string outputFilename = argv[2];

    std::string sourceCode = readSourceFile(inputFilename);

    Lexer lexer(sourceCode);
    std::vector<Token> tokens = lexer.tokenize();

    Parser parser(tokens);
    std::unique_ptr<Node> ast = parser.parse();

    CodeGenerator codegen("main_module");
//    codegen.generateRuntime();
    codegen.generate(ast.get());

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    std::string error;
    auto targetTriple = llvm::sys::getDefaultTargetTriple();
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);

    if (!target)
    {
        std::cerr << error << std::endl;
        return 1;
    }

    llvm::TargetOptions opt;
    auto targetMachine = target->createTargetMachine(targetTriple, "generic", "", opt, llvm::Reloc::PIC_);

    auto module = codegen.getModule();
    module->setDataLayout(targetMachine->createDataLayout());
    module->setTargetTriple(targetTriple);

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
    modulePM.run(*module, MAM);

    module->print(llvm::outs(), nullptr);

    std::error_code ec;
    llvm::raw_fd_ostream dest(outputFilename.c_str(), ec, llvm::sys::fs::OF_None);

    if (ec)
    {
        std::cerr << "Could not open output file '" << outputFilename << "': " << ec.message() << std::endl;
        return 1;
    }

    llvm::legacy::PassManager legacyPM;

    if (targetMachine->addPassesToEmitFile(legacyPM, dest, nullptr, llvm::CodeGenFileType::ObjectFile))
    {
        std::cerr << "Failed to emit object file" << std::endl;
        return 1;
    }

    legacyPM.run(*module);
    dest.flush();

    std::cout << "Successfully compiled '" << inputFilename << "' to object file '" << outputFilename << "'" << std::endl;
    return 0;
}