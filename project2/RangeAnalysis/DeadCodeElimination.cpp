#include <unordered_map> 
#include <vector>
#include <algorithm>
#include <string>

#include "llvm/ADT/Statistic.h"
#include "llvm/Pass.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/User.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Local.h"
#include "RangeAnalysis.h"

using namespace llvm;
// Used to enable the stats computing. Comment the below line to disable it
#define STATS
#define 	DEBUG_TYPE   "DeadCodeElimination"
STATISTIC(InstructionsEliminated, "Number of instructions eliminated");
STATISTIC(BasicBlocksEliminated,  "Number of basic blocks entirely eliminated");

namespace {
class RADeadCodeElimination : public llvm::FunctionPass {
private:

  void replaceConditionalBranch(BranchInst *br, int successorIndex){
    BranchInst* New = BranchInst::Create(br->getSuccessor(successorIndex));
    ICmpInst *cond = dyn_cast<ICmpInst>(br->getCondition());
    ReplaceInstWithInst(br, New);
    RecursivelyDeleteTriviallyDeadInstructions(cond);
  }

  int highestOnesChain(BinaryOperator *bo, InterProceduralRA<Cousot> &ra){
    Range range1 = ra.getRange(bo->getOperand(0));
    Range range2 = ra.getRange(bo->getOperand(1));

    uint64_t lower1 = (*range1.getLower().getRawData());
    uint64_t lower2 = (*range2.getLower().getRawData());
    if(!range1.isUnknown() && range1.getLower() == range1.getUpper()){
      uint64_t op1_ones = 0;
      while(lower1&1){
        op1_ones = (op1_ones << 1) | 1;
        lower1 = lower1 >> 1;
      }
      if(!range2.isUnknown() && op1_ones >= (*range2.getUpper().getRawData())){
        return 1;
      }
      return -1;
      
    } else if(!range2.isUnknown() && range2.getLower() == range2.getUpper()){
      uint64_t op2_ones = 0;
      while(lower2&1){
        op2_ones = (op2_ones << 1) | 1;
        lower2 = lower2 >> 1;
      }
      if(!range1.isUnknown() && op2_ones >= (*range1.getUpper().getRawData())){
        return 0;
      }
      return -1;
    }
    return -1;
  }


  bool solveBranchInstruction(BranchInst* br, InterProceduralRA<Cousot> &ra) {
    if (br->isUnconditional()) return false;
    ICmpInst *I = dyn_cast<ICmpInst>(br->getCondition());
    if(!I) return false;
    Range range1 = ra.getRange(I->getOperand(0));
    Range range2 = ra.getRange(I->getOperand(1));
    // range1.print(errs());
    // range2.print(errs());
    // I->dump();
    // errs() << '\n';
    
    bool has_changed = false;
    switch (I->getPredicate()) {
      case CmpInst::ICMP_EQ:
        if (range1.getLower() == range1.getUpper() == 
            range2.getLower() == range2.getUpper()) {
          replaceConditionalBranch(br, 0);
          has_changed = true;
        } else {
            if(I->isSigned()) {
              (range1.getUpper().sle(range2.getLower()));
              replaceConditionalBranch(br, 1);
              has_changed = true;
            } else {
              (range2.getUpper().sle(range1.getLower()));
              replaceConditionalBranch(br, 1);
              has_changed = true;
            }
        }
        break;

      case CmpInst::ICMP_UGT:
        if (range1.getLower().ugt(range2.getUpper())) {
          replaceConditionalBranch(br, 0);
          has_changed = true;
        } else if(range1.getUpper().ule(range2.getLower())) {
          replaceConditionalBranch(br, 1);
          has_changed = true;
        }
        break;

      case CmpInst::ICMP_UGE:
        if (range1.getLower().uge(range2.getUpper())) {
          replaceConditionalBranch(br, 0);
          has_changed = true;
        } else if(range1.getUpper().ult(range2.getLower())) {
          replaceConditionalBranch(br, 1);
          has_changed = true;
        }
        break;

      case CmpInst::ICMP_ULT:
        if (range1.getUpper().ult(range2.getLower())) {
          replaceConditionalBranch(br, 0);
          has_changed = true;
        } else if(range1.getLower().uge(range2.getUpper())) {
          replaceConditionalBranch(br, 1);
          has_changed = true;
        }
        break;

      case CmpInst::ICMP_ULE:
        if (range1.getUpper().ule(range2.getLower())) {
          replaceConditionalBranch(br, 0);
          has_changed = true;
        } else if(range1.getLower().ugt(range2.getUpper())) {
          replaceConditionalBranch(br, 1);
          has_changed = true;
        }
        break;

      case CmpInst::ICMP_SGT:
        //This code is always true
        if (range1.getLower().sgt(range2.getUpper())) {
          replaceConditionalBranch(br, 0);
          has_changed = true;
        } else if(range1.getUpper().sle(range2.getLower())) {
          replaceConditionalBranch(br, 1);
          has_changed = true;
        }
        break;

      case CmpInst::ICMP_SGE:
        if (range1.getUpper().sge(range2.getLower())) {
            replaceConditionalBranch(br, 0);
            has_changed = true;
        } else if(range1.getLower().slt(range2.getUpper())) {
          replaceConditionalBranch(br, 1);
          has_changed = true;
        }
        break;

      case CmpInst::ICMP_SLT:
        //This code is always true
        if (range1.getUpper().slt(range2.getLower())) {
          replaceConditionalBranch(br, 0);
          has_changed = true;
        } else if(range1.getLower().sge(range2.getUpper())) {
          replaceConditionalBranch(br, 1);
          has_changed = true;
        }
        break;

        case CmpInst::ICMP_SLE:
        if (range1.getUpper().sle(range2.getLower())) {
          replaceConditionalBranch(br, 0);
          has_changed = true;
        } else if(range1.getLower().sgt(range2.getUpper())) {
          replaceConditionalBranch(br, 1);
          has_changed = true;
        }
        break;

      default: break;
    }
    return has_changed;
  }

  bool hasConstantRange(Instruction *I, InterProceduralRA<Cousot> &ra){
    const Value *v = &(*I);
    Range r = ra.getRange(v);
    return !r.isUnknown() && r.getLower() == r.getUpper();
  }

public:
  static char ID;
  RADeadCodeElimination() : FunctionPass(ID) {}
  virtual ~RADeadCodeElimination() {}
  

  virtual bool runOnFunction(Function &F) {
    InterProceduralRA<Cousot> &ra = getAnalysis<InterProceduralRA<Cousot>>();
    bool has_changed = false;
    
    // Remove dead basic blocks
    BasicBlocksEliminated = F.size();
    InstructionsEliminated = 0;
    for (BasicBlock &bb: F) {
      InstructionsEliminated += bb.size();
      if (BranchInst *br = dyn_cast<BranchInst>(--(bb.end()))){
        has_changed |= solveBranchInstruction(br, ra);
      }
    }
    removeUnreachableBlocks(F);


    // Collects the "single" incoming values in phi-functions
    // Collects variables that are constants
    for (BasicBlock &bb: F){
      std::vector<PHINode*> phiNodesToDelete;
      std::vector<Instruction*> constantInstructions;
      std::vector<std::pair<BinaryOperator*, int>> bitwiseAnd;
      for (Instruction &I: bb) {
        if (PHINode *PN = dyn_cast<PHINode>(&I)) {
          if(PN->getNumIncomingValues() == 1) {
            phiNodesToDelete.push_back(PN);
          }
        } else if(hasConstantRange(&I, ra)){
          constantInstructions.push_back(&I);
        } else if (BinaryOperator *bo = dyn_cast<BinaryOperator>(&I)) {
          // check bitwise AND operator
          if(std::string(bo->getOpcodeName()) == "and") {
            int operandIndex = highestOnesChain(bo, ra);
            // if a bitwise operator with a chain of 1's cannot be optimized,
            // checks if this instruction has a constant to be propagated
            if (operandIndex >= 0) bitwiseAnd.push_back({bo, operandIndex});
          }
        }
      }

      // Replaces phi-functions that have "single" incoming values to the proper value
      has_changed |= !phiNodesToDelete.empty() || !constantInstructions.empty() || !bitwiseAnd.empty();
      for(int i=0; i<phiNodesToDelete.size(); ++i){
        phiNodesToDelete[i]->replaceAllUsesWith(phiNodesToDelete[i]->getIncomingValue(0));
        RecursivelyDeleteTriviallyDeadInstructions(phiNodesToDelete[i]);
      }

      // Constant propagation
      for(int i=0; i<constantInstructions.size(); ++i){
        Value *v = constantInstructions[i];
        Range r = ra.getRange(v);
        if (IntegerType *IT = dyn_cast<IntegerType>(v->getType())) {
          ConstantInt* CT = ConstantInt::get(v->getContext(), r.getUpper());
          v->replaceAllUsesWith(CT);
          RecursivelyDeleteTriviallyDeadInstructions(v);
        }
      }

      // Replace bitwise AND operations when operands has a chain of 1's 
      for(int i=0; i<bitwiseAnd.size(); ++i){
        Value *v = bitwiseAnd[i].first->getOperand(bitwiseAnd[i].second);
        bitwiseAnd[i].first->replaceAllUsesWith(v);
        RecursivelyDeleteTriviallyDeadInstructions(bitwiseAnd[i].first);
      }

      // Merges basic blocks
      if (BasicBlock *pred = bb.getSinglePredecessor())
        if (BasicBlock *succ = pred->getSingleSuccessor())
            MergeBasicBlockIntoOnlyPred(succ);
    }
    
    // computes statistics
    BasicBlocksEliminated -= F.size();
    for (Function::iterator bb = F.begin(), bbEnd = F.end(); bb != bbEnd; ++bb){
      InstructionsEliminated -= bb->size();
    }
    return has_changed;
  }

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AU.addRequired<InterProceduralRA<Cousot> >();
  }
};
}

char RADeadCodeElimination::ID = 0;
static RegisterPass<RADeadCodeElimination> X("ra-dead-code-elimination",
                                "Dead code elimination that uses RangeAnalysis");
