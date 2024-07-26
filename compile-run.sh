#!/bin/bash

# Add include paths manually
INCLUDE_PATHS="-isystem /usr/include/c++/11 -isystem /usr/include/x86_64-linux-gnu/c++/11 -isystem /usr/include/c++/11/backward -isystem /usr/lib/gcc/x86_64-linux-gnu/11/include -isystem /usr/local/include -isystem /usr/include/x86_64-linux-gnu -isystem /usr/include"

# Add the library path
LIBRARY_PATH="-L/usr/lib/x86_64-linux-gnu"

# Compile the C++ code with clang++
clang++ -v $(llvm-config --cxxflags --ldflags --system-libs --libs core) $INCLUDE_PATHS $LIBRARY_PATH -fexceptions -o EvaLLVM EvaLLVM.cpp

# Run the compiled executable
./EvaLLVM

# Run the LLVM intermediate representation if required
lli ./out.ll

# Print the exit status of the last command
echo $?

printf "\n"
