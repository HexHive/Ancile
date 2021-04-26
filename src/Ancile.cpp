#include "llvm/ADT/PriorityQueue.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Analysis/CFGPrinter.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/TypeBuilder.h"
#include "llvm/IR/ValueHandle.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/TableGen/Error.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/IR/InstrTypes.h"

#include <cxxabi.h>
#include <fstream>
#include <iostream>
#include <queue>
#include <set>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/file.h>
#include <vector>

using namespace llvm;
using namespace std;

namespace {

struct Ancile : public ModulePass {

  static char ID;
  LLVMContext *Context;

  Ancile() : ModulePass(ID) {}

  bool doInitialization(Module &M) { return true; }
  bool doFinalization(Module &M) { return false; }
  virtual bool runOnModule(Module &M) {

    LLVMContext &Ctx = M.getContext();
    Type *VoidTy = Type::getVoidTy(Ctx);
    Type *VoidPtrTy = Type::getInt8PtrTy(Ctx);
    uint64_t count_TotalFunc = 0;
    uint64_t count_NonEmptyFunc = 0;
    uint64_t count_DirectCall = 0;
    std::ofstream TotalFuncLog, NonEmptyFuncLog, DirectCallLog;

    std::string funcName;
    llvm::raw_string_ostream rso(funcName);
    llvm::Function *FailedFunc;
    for (Function &Func : M.functions()) {
      if (strstr(Func.getName().data(), "baz") != NULL)
        FailedFunc = &Func;
    }
 
    Function *failedFn = nullptr;
    std::vector<Type *> Doubles(2, Type::getDoubleTy(Ctx));
    FunctionType *FT = FunctionType::get(VoidTy, true);
    if(!(M.getFunction("FailedFunc")) ){
    failedFn =
        Function::Create(FT, Function::InternalLinkage, "FailedFunc", &M);
    BasicBlock *BB = BasicBlock::Create(Ctx, "entry", failedFn);
    IRBuilder<> builder(Ctx);
    builder.SetInsertPoint(BB);
    builder.CreateRetVoid();
  }
    char *path_list = "/pat/to/list.txt";
    std::set<std::string> whitelist;
    std::ifstream filePath(path_list, std::ios_base::in);
    std::string temp;
    while (getline(filePath, temp, '\n')) {
      whitelist.insert(temp);
    }

   for (Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
      if (!F->empty()) {
        for (Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB) {
          BasicBlock &b = *BB;
          for (BasicBlock::iterator i = b.begin(), ie = b.end(); i != ie; ++i) {
            if (CallInst *call_inst = dyn_cast<CallInst>(&*i)) {
              if ((call_inst->getCalledFunction()) != nullptr && !isa<InlineAsm>(call_inst->getCalledValue())) { // direct call
                if (!(call_inst->getCalledFunction()->isIntrinsic())) {
                 if (DILocation *Loc = call_inst->getDebugLoc()) {
                  if (whitelist.find(call_inst->getCalledFunction()->getName().str().c_str()) == whitelist.end()) {
                  
		    AttributeList ptr;
		    call_inst->setAttributes(ptr);
		    call_inst->setCalledFunction(failedFn);
		    unsigned loop_count = call_inst->getNumArgOperands();
		    unsigned i = 0;
		    for(i=0; i < loop_count; i++){
		     if (i == 0) { 
		     Value *param = ConstantInt::get(Type::getInt64Ty(Ctx), (loop_count -1));
	              call_inst->setArgOperand(i,param);	     
		     }
		     else {
		       Value *param = ConstantInt::get(Type::getInt64Ty(Ctx), 1);
		       
                       call_inst->setArgOperand(i,param);
		     }
		    		    
		    }
		    Value *param = ConstantInt::get(Type::getInt64Ty(Ctx), 1);
 
                  } // end of whitelist condition check
		 }// debugloc
                }   // end of intrinsic check
              }     // end if indirect call
            }       // end if call inst
          }         // end loop over inst in bb
        }           // end loop over bb in function
      }             // end if non empty function
    }               // end loop over function in module

    return true;
  } // end run on module

  virtual bool runOnFunction(Function &F) { return false; }
};

} // namespace

// register pass
char Ancile::ID = 0;

INITIALIZE_PASS(Ancile, "Ancile", "Ancile", false, false);

ModulePass *llvm::createAncilePass() { return new Ancile(); }
