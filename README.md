# Ancile

Steps to build:


***Step 1:***
clone the repo

***Step 2:***
Now in your llvm working directory, do the following steps

```
  - rm /path/to/llvm/tools/clang/lib/Driver/ToolChains/CommonArgs.cpp  
    ln -s /path/to/Ancile/src/CommonArgs.cpp /path/to/llvm/tools/clang/lib/Driver/ToolChains/CommonArgs.cpp 

  - rm /path/to/llvm/tools/clang/lib/CodeGen/BackendUtil.cpp
    ln -s /path/to/Ancile/src/BackendUtil.cpp /path/to/llvm/tools/clang/lib/CodeGen/BackendUtil.cpp
    
  - rm /path/to/llvm/tools/clang/lib/CodeGen/CGClass.cpp
    ln -s /path/to/Ancile/src/CGClass.cpp /path/to/llvm/tools/clang/lib/CodeGen/CGClass.cpp
    
  - rm /path/to/llvm/tools/clang/lib/CodeGen/CGCall.cpp
    ln -s /path/to/Ancile/src/CGCall.cpp /path/to/llvm/tools/clang/lib/CodeGen/CGCall.cpp
    
  - rm /path/to/llvm/tools/clang/lib/CodeGen/CGExpr.cpp
    ln -s /path/to/Ancile/src/CGExpr.cpp /path/to/llvm/tools/clang/lib/CodeGen/CGExpr.cpp

  - rm /path/to/llvm/tools/clang/include/clang/Basic/Sanitizers.def
    ln -s /path/to/Ancile/src/Sanitizers.def /path/to/llvm/tools/clang/include/clang/Basic/Sanitizers.def
    
  - rm /path/to/llvm/tools/clang/include/clang/Driver/SanitizerArgs.h
    ln -s /path/to/Ancile/src/SanitizerArgs.h /path/to/llvm/tools/clang/include/clang/Driver/SanitizerArgs.h
    
  - rm /path/to/llvm/include/llvm/InitializePasses.h
    ln -s /path/to/Ancile/src/InitializePasses.h /path/to/llvm/include/llvm/InitializePasses.h
    
  - rm /path/to/llvm/lib/Transforms/Instrumentation/CMakeLists.txt
    ln -s /path/to/Ancile/src/CMakeLists.txt /path/to/llvm/lib/Transforms/Instrumentation/CMakeLists.txt
    
  - rm /path/to/llvm/lib/Transforms/IPO/LowerTypeTests
    ln -s /path/to/Ancile/src/LowerTypeTests.cpp /path/to/llvm/lib/Transforms/IPO/LowerTypeTests.cpp

  - ln -s /path/to/Ancile/src/Ancile.cpp path/to/llvm/lib/Transforms/Instrumentation/Ancile.cpp
  - ln -s /path/to/Ancile/src/HexCFI.cpp path/to/llvm/lib/Transforms/Instrumentation/HexCFI.cpp
```
***Step 3:***
a. Now add the runtime library
```
ln -s /path/to/Ancile/runtime/ancile  /path/to/llvm/projects/compiler-rt/lib/ancile
ln -s /path/to/Ancile/runtime/hexcfi  /path/to/llvm/projects/compiler-rt/lib/hexcfi

```

b. Add the following line in the CMakeLists.txt at /path/to/llvm/projects/compiler-rt/lib/
```
add_subdirectory(ancile)
add_subdirectory(hexcfi)
```

***Step 4:*** cmake option for build
cmake -GNinja \
-DCMAKE_BUILD_TYPE=Release \
-DCMAKE_C_COMPILER=clang \
-DCMAKE_CXX_COMPILER=clang++ \
-DLLVM_ENABLE_ASSERTIONS=ON \
-DLLVM_BUILD_TESTS=OFF \
-DLLVM_BUILD_EXAMPLES=OFF \
-DLLVM_INCLUDE_TESTS=OFF \
-DLLVM_INCLUDE_EXAMPLES=OFF \
-DBUILD_SHARED_LIBS=on \
-DLLVM_TARGETS_TO_BUILD="X86" \
-DCMAKE_C_FLAGS="-fstandalone-debug" \
-DCMAKE_CXX_FLAGS="-fstandalone-debug" \
../llvm

***Step 5:***
Build command

```
ninja
```
