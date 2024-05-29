/**
 * Eva LLVM executable.
 */
#include <string>
#include "./EvaLLVM.h" // Assuming EvaLLVM.h is in the same directory or include path

int main(int argc, char const *argv[]) {
    /**
     * Program to execute.
     * Assuming the program is a simple expression or similar.
     * Replace this with actual program content or load from file/arguments as needed.
     */
    //        (printf "Value: %d\n" 42)   

    std::string program = R"( 

        "Hello"   

    )"; 
 
    /**
     * Compiler instance.
     */
    EvaLLVM vm;

    /**
     * Generate LLVM IR and execute the program..
     */
    vm.exec(program);

    return 0;
}
