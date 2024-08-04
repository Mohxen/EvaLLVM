#ifndef EvaLLVM_h
#define EvaLLVM_h

#include <iostream>
#include <regex>
#include <string>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

#include "./Environment.h"
#include "parser/EvaParser.h"

using syntax::EvaParser;

/**
 * Environment type.
 */
using Env = std::shared_ptr<Environment>;

class EvaLLVM {
public:
    EvaLLVM() : parser(std::make_unique<EvaParser>()){
        moduleInit();
        setupExternalFunctions();
        setupGlobalEnvironment();
    }

    // Executes a program.
    void exec(const std::string& program) {
        // 1. Parse the program
        auto ast = parser->parse("(begin " + program + ")");

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
        fn = createFunction("main", llvm::FunctionType::get(/* return type */ builder->getInt32Ty(),/* vararg */ false), GlobalEnv);
        
        createGlobalVar("VERSION", builder->getInt32(42));

        // 2. compile main body
        auto result = gen(ast, GlobalEnv);

        // 3. cast to i32 to return from main
        // auto i32Result =
        //     builder->CreateIntCast(result, builder->getInt32Ty(), true);

        // 4. return
        // builder->CreateRet(i32Result);
        // 4. just return zero
        builder->CreateRet(builder->getInt32(0));
    }
    /* main compile loop */

    llvm::Value* gen(const Exp& exp, Env env){ 
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
                // return builder->getInt32(0);
                /**
                 * Boolean
                 */
                if (exp.string == "true" || exp.string == "false"){
                    return builder->getInt1(exp.string == "true" ? true : false);
                } else {
                    // Variable
                    auto varName = exp.string; 
                    auto value = env->lookup(varName);
                    // 1. Local Vars: (TODO)

                    // 2. Global vars:
                    if(auto globalVar = llvm::dyn_cast<llvm::GlobalVariable>(value)) {
                        return builder->CreateLoad(globalVar->getInitializer()->getType(), 
                                                    globalVar, varName.c_str());
                    }
                }
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
                // ------------------------------------------
                // Variable declaration: (var x (+ y 10))

                if (op == "var"){
                    // TODO: handle generic value 

                    auto varName = exp.list[1].string;

                    // Initializer:
                    auto init = gen(exp.list[2], env);

                    return createGlobalVar(varName, (llvm::Constant*) init);
                }
                // ------------------------------------------
                // Blocks: (begin <expressions>)
                else if (op == "begin"){
                    // Compile each expression within the block.
                    // Result is the last evaluated expression.
                    llvm::Value* blockRes;
                    for (auto i = 1; i < exp.list.size(); i++){
                        // Generate expression code.
                        blockRes = gen(exp.list[i], env); // TODO: local block env!
                    }
                    return blockRes;    
                }


                // ------------------------------------------
                // printf extern function:
                //
                // printf ("Value: %d" 42)
                //

                if (op == "printf") {
                    auto printfFn = module->getFunction("printf");
                    std::vector<llvm::Value*> args{};
                    for (auto i = 1; i < exp.list.size(); i++)
                    {
                        args.push_back(gen(exp.list[i], env));
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

    /**
     * Creates a global variable.
     */
    llvm::GlobalVariable* createGlobalVar(const std::string& name, 
                                        llvm::Constant* init){
        module->getOrInsertGlobal(name, init->getType());
        auto variable = module->getNamedGlobal(name);
        variable->setAlignment(llvm::MaybeAlign(4));
        variable->setConstant(false);
        variable->setInitializer(init);
        return variable;
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
    llvm::Function* createFunction(const std::string& fnName, llvm::FunctionType* fnType, Env env){

        // function prototype might already be defined
        auto fn = module->getFunction(fnName);
        
        // if not, allocate the function
        if (fn == nullptr){
            fn = createFunctionProto(fnName, fnType, env);
        }

        createFunctionBlock(fn);
        return fn;
    }

    llvm::Function* createFunctionProto(const std::string& fnName, llvm::FunctionType* fnType, Env env){

        auto fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, fnName, *module);

        verifyFunction(*fn);

        // Install in the enviornment:
        env->define(fnName, fn);

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
     * Setup Global Environment.
     */
    void setupGlobalEnvironment(){
        std::map<std::string, llvm::Value*> globalObject {
            {"VERSION", builder->getInt32(42)}, 
        };
        std::map<std::string, llvm::Value*> globalRec{};
        
        for (auto& entry : globalObject){
            globalRec[entry.first] = 
                createGlobalVar(entry.first, (llvm::Constant*)entry.second);
        }

        GlobalEnv = std::make_shared<Environment>(globalRec, nullptr);
    }

    /**
     * Parser.
    */
    std::unique_ptr<EvaParser> parser;

    /**
     * Global Environment (symbol table).
     */
    std::shared_ptr<Environment> GlobalEnv;

    /**
     * currently compiling function.
     */
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

