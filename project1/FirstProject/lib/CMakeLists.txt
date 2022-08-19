find_package(Clang REQUIRED)

set(CMAKE_CXX_FLAGS "-Wall -fno-rtti")
include_directories(${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})


add_library(FirstProject MODULE FirstProject.cpp)
