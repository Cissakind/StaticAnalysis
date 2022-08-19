The pass is compiled outside the LLVM build tree.

Compile:

- edit the llvmBuildDir inside the file setup with 
  your llvm build dir

- type make


Run command:

opt -enable-new-pm=0 -load build/lib/libFirstProject.so -FirstProject file.bc > /dev/null