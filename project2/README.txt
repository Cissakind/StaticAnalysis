Assignment 2:
Group: 
    Cecília Kind 2019054420
    Vinicuis Julião  2018054630

Dependencies:
    cmake 2.20

Compile and Run instructions:
At the same directory of this README file, type:
    make LLVM_INCLUDE_DIRS="<path to LLVM include directory>"
    opt -load build/libRADeadCodeElimination.so -vssa -ra-dead-code-elimination -stats -S <input bytecode file> -o <output bytecode file>

