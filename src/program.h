#pragma once
#include "types.h"
#include "primitive.h"
#include "function.h"
#include <vector>
#include <unordered_map>

// ============================================================
// Program
// ============================================================

struct Program {

  primitive::Sequence bootstrap;
  primitive::Sequence hatstrap;
  std::string entryFunctionName;
  
  std::vector<Function> functions;
  std::unordered_map<std::string, size_t> functionByName;
  std::unordered_map<std::string, Slot> globals;
  std::vector<Function::Block*> globalBlockOrder;

  inline Function& createFunction(std::string name, FunctionSignature sig) {
    assert(!functionByName.contains(name));
    size_t idx = functions.size();
    functionByName[name] = idx;
    functions.push_back(Function{
	.functionIndex = idx,
	.name = std::move(name),
	.frame = FrameLayout{sig.returnType->size()},
	.sig = std::move(sig)
      });
    return functions.back();
  }

  inline Function &function(std::string const &name) {
    return functions.at(functionByName.at(name));
  }

  inline Function const &function(std::string const &name) const {
    return functions.at(functionByName.at(name));
  }

  inline size_t nextGlobalBlockIndex() const {
    return globalBlockOrder.size();
  }

  inline void registerBlock(Function::Block &block) {
    globalBlockOrder.push_back(&block);
  }

  inline int globalVariableFrameSize() const {
    int result = 0;
    for (auto const &[_, slot]: globals) result += slot.size();
    return result;
  }

  inline bool isFunctionDefined(std::string const &f) const {
    return (functionByName.find(f) != functionByName.end());
  }
};
