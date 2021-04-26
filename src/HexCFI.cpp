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

#include <cxxabi.h>
#include <fstream>
#include <iostream>
#include <queue>
#include <set>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/file.h>
#include <vector>

using namespace llvm;
using namespace std;

namespace {

struct HexCFI : public ModulePass {

  static char ID;
  LLVMContext *Context;
  uint64_t ins_count = 0;
  HexCFI() : ModulePass(ID) {}

  bool doInitialization(Module &M) { return true; }
  bool doFinalization(Module &M) { return false; }
  virtual bool runOnModule(Module &M) {

    LLVMContext &Ctx = M.getContext();
    Type *VoidTy = Type::getVoidTy(Ctx);
    Type *VoidPtrTy = Type::getInt8PtrTy(Ctx);

    FunctionType *runFType =
        FunctionType::get(VoidTy, {VoidPtrTy, VoidPtrTy}, false);

    /* This segment is to collect information about all the
       functions within the program */
  /*
    std::ofstream TotalFuncLog, NonEmptyFuncLog, DirectCallLog;

     TotalFuncLog.open("/path/to/file.txt", std::ios_base::app | std::ios_base::out);
     NonEmptyFuncLog.open("/path/to/file/" +
     M.getName().str() + "_nonEmptyFunc.txt", std::ios_base::app |
     std::ios_base::out);

     std::string funcName;
     llvm::raw_string_ostream rso(funcName);

     for (Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {

       TotalFuncLog << F->getName().str()<< "\n";
       if(!F->empty()) {
          //count_NonEmptyFunc++;
          NonEmptyFuncLog << F->getName().str() << "\n";

                        }//end if non empty function
                } //end loop over function in module
     TotalFuncLog.close();
     NonEmptyFuncLog.close();
     DirectCallLog.close();	
 */
    /* End of totalcall log */

    /* Following segment is to collect information about direct calls
      during runtime */

    for (Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
      if (!F->empty()) {
        for (Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB) {
          BasicBlock &b = *BB;
          for (BasicBlock::iterator i = b.begin(), ie = b.end(); i != ie; ++i) {
            if (CallInst *call_inst = dyn_cast<CallInst>(&*i)) {
              if ((call_inst->getCalledFunction()) != nullptr &&
                  !isa<InlineAsm>(call_inst->getCalledValue())) { // direct call

                if (!(call_inst->getCalledFunction()->isIntrinsic())) {

                  Value *target_val = call_inst->getCalledValue();
                  IRBuilder<> Builder(call_inst);
                  if (DILocation *Loc = call_inst->getDebugLoc()) {
                    unsigned line = Loc->getLine();
                    unsigned col = Loc->getColumn();
                    char call_info[500];

                    sprintf(call_info, "%s:%u:%u", M.getName().str().c_str(),
                            line, col);
                    llvm::StringRef stref(call_info);

                    Constant *c_info =
                        llvm::ConstantDataArray::getString(Ctx, stref, true);
                    char Dfunc_name[500];
		    sprintf(Dfunc_name, "%s", dyn_cast<Function>(call_inst->getCalledValue()->stripPointerCasts())->getName().str().c_str());
                    llvm::StringRef stref2(Dfunc_name);

                    Constant *c_info2 =
                        llvm::ConstantDataArray::getString(Ctx, stref2, true);

                    GlobalVariable *infoGlobal = new GlobalVariable(
                        M, c_info->getType(), true,
                        llvm::GlobalValue::PrivateLinkage, c_info, "", nullptr,
                        llvm::GlobalVariable::NotThreadLocal, 0);
                    auto cast =
                        Builder.CreatePointerCast(infoGlobal, VoidPtrTy);
                    GlobalVariable *infoGlobal2 = new GlobalVariable(
                        M, c_info2->getType(), true,
                        llvm::GlobalValue::PrivateLinkage, c_info2, "", nullptr,
                        llvm::GlobalVariable::NotThreadLocal, 0);
                    auto cast2 =
                        Builder.CreatePointerCast(infoGlobal2, VoidPtrTy);

                    Constant *GInit = M.getOrInsertFunction("target", runFType);
                    Builder.CreateCall(GInit, {cast, cast2});
                    //===================================================================
                  } // end if debug loc
                }   // intrinsic
              }     // end if indirect call
            }       // end if call inst
          }         // end loop over inst in bb
        }           // end loop over bb in function
      }             // end if non empty function
    }               // end loop over function in module

    /* End of logging direct call */

    /* This segment is to collect instrument indirect calls */

    /* 		for (Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
        if(!F->empty()) {
                  for (Function::iterator BB = F->begin(), E = F->end(); BB !=
       E; ++BB) { BasicBlock &b = *BB; for (BasicBlock::iterator i = b.begin(),
       ie = b.end(); i != ie; ++i) { if (CallInst *call_inst =
       dyn_cast<CallInst>(&*i)) { if ((call_inst->getCalledFunction()) ==
       nullptr && !isa<InlineAsm>(call_inst->getCalledValue())) { Value
       *target_val = call_inst->getCalledValue(); IRBuilder<>
       Builder(call_inst);

                                                                          if
       (target_val->getType() != VoidPtrTy) target_val =
       Builder.CreateBitCast(target_val, VoidPtrTy);

                                                                          if
       (DILocation *Loc = call_inst->getDebugLoc()) {
                                                                                  //IRBuilder<> Builder(call_inst);	;
                                                                          unsigned
       line = Loc->getLine(); unsigned col = Loc->getColumn();
                                                                                  //StringRef File = Loc->getFilename();
                                                                                  char call_info[100];

                                                                                  sprintf(call_info,"%s:%u:%u",M.getName().str().c_str(), line, col);
                                                                                  llvm::StringRef stref(call_info);

                                                                                  Constant *c_info = llvm::ConstantDataArray::getString(Ctx, stref, true);
                                                                                  GlobalVariable* infoGlobal = new GlobalVariable(M, c_info->getType(), true, llvm::GlobalValue::PrivateLinkage, c_info,
                          "", nullptr, llvm::GlobalVariable::NotThreadLocal, 0);
                                                                                  auto cast = Builder.CreatePointerCast(infoGlobal, VoidPtrTy);

                                                                                  Constant *GInit = M.getOrInsertFunction("target", runFType);
                                                                                  Builder.CreateCall(GInit, {cast, target_val});
                                                                          }
       //end if debug loc } //end if indirect call } //end if call inst } //end
       loop over inst in bb } //end loop over bb in function
                          }//end if non empty function
                  } //end loop over function in module
  */
    /* End of indirect log */
    return true;
  } // end run on module

  virtual bool runOnFunction(Function &F) { return false; }
};

} // namespace

// register pass
char HexCFI::ID = 0;

INITIALIZE_PASS(HexCFI, "HexCFI", "HexCFI", false, false);

ModulePass *llvm::createHexCFIPass() { return new HexCFI(); }
