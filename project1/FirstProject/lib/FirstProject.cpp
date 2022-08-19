#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include <map>
#include <sstream>
#include <fstream>
using namespace llvm;
using namespace std;

struct block{
  string label = "";
  vector<string> instructions;
  vector<string> targets;
};

void gen_dot(string func_name, map<string, block> name2block, string filename);

namespace {

  struct FirstProject : public FunctionPass {
    static char ID; 
    FirstProject() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
      string filename = F.getParent()->getSourceFileName();
      int bbCounter = 0;
      int numSuccessors;
      map<string, block> name2block;

      // iterate basic blocks  
      for(auto &BB : F) 
      {
        block bb;
        string block_name = BB.getName().str();

        // iterate instructions
        for (auto &I : BB)
        {
          // get name > opcode > operands > targets

          string instruction;
          raw_string_ostream inst(instruction);
          string name = I.getName().str();

          // instruction has name
          if(name != "")
            inst << "%" << name << " = ";
          
          // no name: get aux name
          else
          {
            string no_name;
            raw_string_ostream aux_name(no_name);
            I.printAsOperand(aux_name, false);
            if((aux_name.str())[0] == '%')
              inst << aux_name.str() << " = ";
          }
          
          // opcode
          inst << I.getOpcodeName() << "  ";

          //iterate operands
          for (int i = 0; i < I.getNumOperands(); i++)
          {            
            I.getOperand(i)->printAsOperand(inst , false);
            inst << " , ";

            if(i == I.getNumOperands() - 1)
            {
              inst.str().pop_back();
              inst.str().pop_back();
            }             
          }

          // targets
          if(I.isTerminator())
          {
            numSuccessors = I.getNumSuccessors();

            for(int i = 0; i < numSuccessors; i++)
              bb.targets.push_back(I.getSuccessor(i)->getName().str());
          }

          bb.instructions.push_back(inst.str());
        }

        // add new block
        bb.label = "AA" + to_string(bbCounter);
        name2block[block_name] = bb;
        bbCounter++;
      
      }

      //generate .dot
      gen_dot(F.getName().str(), name2block, filename);
      return false;
    }
  };
}

char FirstProject::ID = 0;
static RegisterPass<FirstProject> X("FirstProject", "FirstProject Pass");

void gen_dot(string func_name, map<string, block> name2block, string filename)
{

  stringstream s;

  s << "digraph \"CFG for \'" << func_name << "\' function\" {\n\n";
  for(auto &b : name2block)
  {
    s << b.second.label << " [shape=record, label=";
    s << "\"{" << b.first << ": \\l\\l" << "\n"; 
    
    for(auto &i : b.second.instructions)
      s << i << " \\l" << "\n";

    s << "}\"];";
    s << "\n";
  
    for(auto &t : b.second.targets)
      s << b.second.label << "->" << name2block[t].label << "\n";

    s << "\n";
  }

  s << "}";

  ofstream myfile;
  myfile.open (func_name + ".dot");
  myfile << s.str();
  myfile.close();
}
