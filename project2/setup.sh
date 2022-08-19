export LLVM="$HOME/llvm-project/build"
export LLVM_INCLUDE_DIRS="$LLVM/include/llvm"
export PATH="$PATH:$LLVM/bin"

# cmake -DLLVM_INCLUDE_DIRS=$LLVM_INCLUDE_DIRS -G "Unix Makefiles" -B build/ .
# cmake --build build
# $LLVM/bin/opt -load build/libRADeadCodeElimination.so -vssa -ra-dead-code-elimination -stats -S examples/ex1.bc -o examples/out1.bc