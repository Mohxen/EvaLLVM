#ifndef EvaLLVM_h
#define EvaLLVM_h

#include <iostream>
#include <regex>
#include <string>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

#include "src/parser/EvaParser.h"

using syntax::EvaParser;
class EvaLLVM {
public:
    EvaLLVM() : parser(std::make_unique<EvaParser>()){
        moduleInit();
        setupExternalFunctions();
    }

    // Executes a program.
    void exec(const std::string& program) {
        // 1. Parse the program
        auto ast = parser->parse(program);

        // 2. Compile to LLVM IR
        //compile(ast);
        compile(ast);

        //Print generated code
        module->print(llvm::outs(), nullptr);

        std::cout << "\n";

        // 3. Save module IR to file
        saveModuleToFile("./out.ll"); 
    }

private:

    void compile(const Exp& ast){
        // 1. create main function
        fn = createFunction("main", llvm::FunctionType::get(/* return type */ builder->getInt32Ty(),
                                                            /* vararg */ false));
        // 2. compile main body
        auto result = gen(ast);

        // 3. cast to i32 to return from main
        // auto i32Result =
        //     builder->CreateIntCast(result, builder->getInt32Ty(), true);

        // 4. return
        // builder->CreateRet(i32Result);
        // 4. just return zero
        builder->CreateRet(builder->getInt32(0));
    }
    /* main compile loop */

    llvm::Value* gen(const Exp& exp){ 
        switch (exp.type) {
            /*
            * Number
            // ------------------------------------------
            */
            case ExpType::NUMBER:
                return builder->getInt32(exp.number);
            /*
            * String
            // ------------------------------------------
            */
            case ExpType::STRING:{
                auto re = std::regex("\\\\n");
                auto str = std::regex_replace(exp.string, re, "\n");
                return builder->CreateGlobalStringPtr(str);
            }
            /*
            * Symbol(variables, operators)
            // ------------------------------------------
            */
            case ExpType::SYMBOL:
                return builder->getInt32(0);
            /*
            * Lists
            // ------------------------------------------
            */
            case ExpType::LIST:
                auto tag = exp.list[0];
            /*
            * Spachial cases
            // ------------------------------------------
            */
            if (tag.type == ExpType::SYMBOL){
                auto op = tag.string;

                if (op == "printf") {
                    auto printfFn = module->getFunction("printf");
                    std::vector<llvm::Value*> args{};
                    for (auto i = 1; i < exp.list.size(); i++)
                    {
                        args.push_back(gen(exp.list[i]));
                    }
                    
                    return builder->CreateCall(printfFn, args);
                }
            }
        }

        // just 42
        //return builder->getInt32(42);

        // string:
        // return builder->CreateGlobalStringPtr("Hello World!\n");
        // auto str = builder->CreateGlobalStringPtr("Hello World!\n");

        // // call to printf
        // auto printfFn = module->getFunction("printf");

        // // args
        // std::vector<llvm::Value*> args{str};

        // return builder->CreateCall(printfFn, args);

        //Unreachable
        return builder->getInt32(0);
    }
    /* define external function from libc++ for printf */
    void setupExternalFunctions() {
        // i8* to substitude for char*, void*, etc 
        auto bytePtrTy = builder->getInt8Ty()->getPointerTo();

        // int printf (const char* format, ...);
        module->getOrInsertFunction("printf", 
            llvm::FunctionType::get(
                /* return type */ builder->getInt32Ty(),
                /* format arg */ bytePtrTy,
                /* vararg */ true
                ));
    }

    // create function
    llvm::Function* createFunction(const std::string& fnName, llvm::FunctionType* fnType){

        // function prototype might already be defined
        auto fn = module->getFunction(fnName);
        
        // if not, allocate the function
        if (fn == nullptr){
            fn = createFunctionProto(fnName, fnType);
        }

        createFunctionBlock(fn);
        return fn;
    }

    llvm::Function* createFunctionProto(const std::string& fnName, llvm::FunctionType* fnType){

        auto fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, fnName, *module);

        verifyFunction(*fn);

        return fn;
    }

    void createFunctionBlock(llvm::Function* fn){
        auto entry = createBB("entry", fn);
        builder->SetInsertPoint(entry);
    }

    llvm::BasicBlock* createBB(std::string name, llvm::Function* fn = nullptr) {
        return llvm::BasicBlock::Create(*ctx, name, fn);
    }

    void saveModuleToFile(const std :: string& fileName) {
        std::error_code errorCode;
        llvm::raw_fd_ostream outLL(fileName, errorCode);
        module->print(outLL, nullptr); 
    }

    void moduleInit() {
        // Open a new context and module.
        ctx = std::make_unique<llvm::LLVMContext>();
        module = std::make_unique<llvm::Module>("EvaLLVM", *ctx);
        // Create a new builder for the module.
        builder = std::make_unique<llvm::IRBuilder<>>(*ctx);
    }

    /**
     * Parser.
    */
    std::unique_ptr<EvaParser> parser;

    llvm::Function* fn;
    // Global LLVM context.
    // It owns and manages the core "global" data of LLVM's core infrastructure,
    // including the type and constant unique tables.
    std::unique_ptr<llvm::LLVMContext> ctx;

    // A Module instance is used to store all the information related to an LLVM module.
    // Modules are the top level container of all other LLVM Intermediate Representation (IR)
    // objects. Each module directly contains a list of global variables, a list of functions,
    // a list of libraries (or other modules) this module depends on, a symbol table,
    // and various data about the target's characteristics.
    // A module maintains a GlobalList object that is used to hold all constant references
    // to global variables in the module. When a global variable is destroyed, it should
    // have no entries in the GlobalList.
    // The main container class for the LLVM Intermediate Representation.
    std::unique_ptr<llvm::Module> module;

    // IR Builder.
    // This provides a uniform API for creating instructions and inserting them into a basic block:
    // either at the end of a BasicBlock, or at a specific iterator location in a block.
    std::unique_ptr<llvm::IRBuilder<>> builder;
};

#endif // EvaLLVM_h

