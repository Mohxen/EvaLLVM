clang++ -v $(llvm-config --cxxflags --ldflags --system-libs --libs core) -fexceptions -o EvaLLVM EvaLLVM.cpp
#clang++ -v -o EvaLLVM $(llvm-config --cxxflags --ldflags --system-libs --libs core) -I/usr/include/c++/11 EvaLLVM.cpp

./EvaLLVM

lli ./out.ll

echo $?

printf "\n"