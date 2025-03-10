#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"

#include <string>
#include <vector>

using namespace llvm;

static int findNode(const std::vector<const Instruction *> nodes,
                    const Instruction *inst) {
  for (unsigned i = 0; i < nodes.size(); i++)
    if (nodes[i] == inst)
      return i;

  return -1;
}

static void findExternalInputs(const Instruction &inst,
                               std::vector<const Instruction *> &nodes) {
  for (const Use &U : inst.operands()) {
    const Instruction *operand = dyn_cast<Instruction>(U.get());
    if (operand
        && operand->getParent() != inst.getParent()
        && std::find(nodes.begin(), nodes.end(),
                     operand) == nodes.end())
      nodes.push_back(operand);
  }
}

static bool isExternalOutput(const Instruction &inst) {
  bool ret = false;
  const BasicBlock *B = inst.getParent();
  for (const Use &U : inst.uses()) {
    const Instruction *I = cast<Instruction>(U.getUser());
    const PHINode *PN = dyn_cast<PHINode>(I);
    if (PN) {
      ret = true;
      if (PN->getParent() == B)
        errs() << "LCD:" << *I << "\n";
    } else if (I->getParent() != B)
      ret = true;
  }
  return ret;
}

static bool isMarked(const Instruction &inst) {
  switch (inst.getOpcode()) {
  case Instruction::Alloca:
  case Instruction::AtomicCmpXchg:
  case Instruction::AtomicRMW:
  case Instruction::Br:
  case Instruction::Call:
  case Instruction::ExtractValue:
  case Instruction::Fence:
  case Instruction::GetElementPtr:
  case Instruction::IndirectBr:
  case Instruction::InsertValue:
  case Instruction::Invoke:
  case Instruction::LandingPad:
  case Instruction::Load:
  case Instruction::PHI:
  case Instruction::Resume:
  case Instruction::Ret:
  case Instruction::Store:
  case Instruction::Switch:
    return true;

  case Instruction::AShr:
  case Instruction::Add:
  case Instruction::And:
  case Instruction::BitCast:
  case Instruction::ExtractElement:
  case Instruction::FAdd:
  case Instruction::FCmp:
  case Instruction::FDiv:
  case Instruction::FMul:
  case Instruction::FPExt:
  // Add other cases as needed
  default:
    return false;
  }
}

static std::string getBlockName(const BasicBlock &B) {
  if (!B.getName().empty())
    return B.getName().str();

  std::string Str;
  raw_string_ostream OS(Str);

  B.printAsOperand(OS, false);
  return OS.str();
}

static uint64_t getFunctionEntryCount(const Function &F) {
  uint64_t count = 0;
  if (F.hasMetadata()) {
    MDNode *node = F.getMetadata("prof");
    MDString *MDS = dyn_cast<MDString>(node->getOperand(0));
    if (MDS) {
      if (MDS->getString().equals("function_entry_count")) {
        ConstantInt *CI =
          mdconst::dyn_extract<ConstantInt>(node->getOperand(1));
        if (CI)
          count = CI->getZExtValue();
      }
    }
  }
  return count;
}

namespace {
  struct BBPrinter : public PassInfoMixin<BBPrinter> {
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
  };
}

PreservedAnalyses BBPrinter::run(Module &M, ModuleAnalysisManager &MAM) {
  std::vector<const Instruction *> nodes, ext_ins, ext_outs;

  for (auto &F : M) {
    if (F.isDeclaration())
      continue;

    auto &FAM = MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
    auto &BFI = FAM.getResult<BlockFrequencyAnalysis>(F);
    int bb_idx = 0;
    for (const auto &B : F) {
      bb_idx++;
      std::string graphName =
        sys::path::filename(M.getModuleIdentifier()).str() +
        "_" + F.getName().str() +
        "_" + getBlockName(B) +
        "." + std::to_string(bb_idx - 1);
      std::string graphFile = "DFG_" + graphName + ".gv";

      std::error_code EC;
      raw_fd_ostream graph(graphFile, EC, sys::fs::OF_None);
      if (EC) {
        errs() << "Error opening file: " << graphFile << "\n";
        continue;
      }

      float bb_freq = static_cast<float>(BFI.getBlockFreq(&B).getFrequency())
        * getFunctionEntryCount(F)
        / BFI.getEntryFreq();

      graph << "digraph \"" << graphName << "\" {" << "\n"
            << "frequency = " << std::to_string(bb_freq) << "\n";

      nodes.clear();
      ext_ins.clear();
      ext_outs.clear();

      // print nodes in the graph, fill marked nodes with red color
      int i_idx = 0;
      for (const auto &inst : B) {
        bool forbidden = isMarked(inst);
        nodes.push_back(&inst);
        findExternalInputs(inst, ext_ins);
        if (isExternalOutput(inst) && !forbidden)
          ext_outs.push_back(&inst);

        graph << "N" << i_idx << "_" << inst.getOpcodeName()
              << " [weight = 1, forbidden = " << forbidden;
        if (forbidden)
          graph << ", style = filled, fillcolor = red";
        graph << "]\n";

        i_idx++;
      }

      // print edges in the graph
      for (int i_idx = 0; i_idx < nodes.size(); i_idx++) {
        const Instruction *inst = nodes[i_idx];
        for (const User *U : inst->users()) {
          const Instruction *user = dyn_cast<Instruction>(U);
          if (user) {
            int u_idx = findNode(nodes, user);
            if (u_idx >= i_idx) {
              if (isMarked(*inst))
                graph << "N" << i_idx << "_" << inst->getOpcodeName()
                      << " -> "
                      << "N" << u_idx << "_" << user->getOpcodeName()
                      << " [color = red];\n";
              else
                graph << "N" << i_idx << "_" << inst->getOpcodeName()
                      << " -> "
                      << "N" << u_idx << "_" << user->getOpcodeName()
                      << ";\n";
            }
          }
        }
      }

      // print ext_in nodes and their edges
      for (int i_idx = 0; i_idx < ext_ins.size(); i_idx++) {
        const Instruction *inst = ext_ins[i_idx];
        graph << "ExtIn" << i_idx << "_" << inst->getOpcodeName()
              << "[style = filled, fillcolor = blue]" << "\n";
        for (const User *U : inst->users()) {
          const Instruction *user = dyn_cast<Instruction>(U);
          if (user) {
            int u_idx = findNode(nodes, user);
            if (u_idx != -1)
              graph << "ExtIn" << i_idx << "_" << inst->getOpcodeName()
                    << " -> "
                    << "N" << u_idx << "_" << inst->getOpcodeName()
                    << " [color = blue];\n";
          }
        }
      }

      // add edges from ext_out nodes to a dummy node
      if (ext_outs.size()) {
        graph << "ExtOut0 [style = filled, fillcolor = blue]\n";
        for (auto &inst : ext_outs)
          graph << "N" << findNode(nodes, inst) << "_" << inst->getOpcodeName()
                << " -> "
                << "ExtOut0 [color = blue];\n";
      }

      graph << "}";
      graph.close();
    }
  }

  return PreservedAnalyses::all();
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION, "BBPrinter", LLVM_VERSION_STRING,
    [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
        [](StringRef Name, ModulePassManager &MPM,
           ArrayRef<PassBuilder::PipelineElement>) {
          if (Name == "bbprinter") {
            MPM.addPass(BBPrinter());
            return true;
          }
          return false;
        }
      );
    }
  };
}