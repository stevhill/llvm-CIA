cmake_minimum_required(VERSION 3.5)
project(llvm-CIA)
set(CMAKE_CXX_STANDARD 11)

find_package(LLVM REQUIRED CONFIG)
list(APPEND  CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(HandleLLVMOptions)
include(AddLLVM)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

add_subdirectory(BBPrinter)
