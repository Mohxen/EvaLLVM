/**
 * Eva LLVM executable.
 */
#include <string>
#include "./src/EvaLLVM.h" // Assuming EvaLLVM.h is in the same directory or include path

int main(int argc, char const *argv[]) {
    /**
     * Program to execute.
     * Assuming the program is a simple expression or similar.
     * Replace this with actual program content or load from file/arguments as needed.
     */
    //(printf "Value: %d\n" 42)
    //"Hello"
    //(printf "False: %d\n\n" false) 
    //(printf "True: %d\n\n" true) 
    //(printf "Version: %d\n\n" version)
    //(var VERSION 42)
            // (var VERSION 42)

        // (begin 
        //     (var VERSION "Hello")
        //     (printf "Version: %s\n\n" VERSION))
    std::string program = R"( 

        (var x 42)

        (begin 
            (var (x string) "Hello")
            (printf "X: %s\n\n" x))
        
        (printf "X: %d\n\n" x)

        (set x 100)

        (printf "X: %d\n\n" x)
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
