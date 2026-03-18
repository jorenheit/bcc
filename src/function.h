#pragma once

#include <vector>
#include <unordered_map>
#include <memory>

#include "data.h"
#include "primitive.h"
#include "frame.h"

// ============================================================
// Function
// ============================================================


struct Function {

  struct Arg {
    enum { Constant, Variable } kind;
    union {
      int value;
      std::string varName;
    };
    
    inline Arg(int value): kind(Constant), value(value) {}
    inline Arg(std::string var): kind(Variable) {
      new (&varName) std::string(std::move(var));
    }
    inline Arg(Arg const &other):
      kind(other.kind)
    {
      if (kind == Constant) value = other.value;
      else varName = other.varName;
    }

    ~Arg(){
      if (kind == Variable) varName.~basic_string();
    }    
  };
  
  struct Block {
    size_t globalBlockIndex = 0;
    size_t parentFunctionIndex = 0;
    std::string name;
    std::string id;
    primitive::Sequence code;
  };
  
  size_t functionIndex = 0;
  std::string name;
  FrameLayout frame;
  FunctionSignature sig;
  
  std::vector<std::unique_ptr<Block>> blocks;
  std::unordered_map<std::string, size_t> blockByName;

  size_t entryBlockIndex = 0;

  inline Block &createBlock(std::string blockName, size_t globalBlockIndex) {
    assert(!blockByName.contains(blockName));
    blockByName[blockName] = blocks.size();
    blocks.push_back(std::make_unique<Block>());

    Block &b = *blocks.back();
    b.globalBlockIndex = globalBlockIndex;
    b.parentFunctionIndex = functionIndex;
    b.name = std::move(blockName);
    b.id = name + "." + b.name;
    return b;
  }

  inline Block &block(std::string const& name) {
    return *blocks.at(blockByName.at(name));
  }

  inline Block const &block(std::string const& name) const {
    return *blocks.at(blockByName.at(name));
  }

  inline bool isBlockDefined(std::string const &b) const {
    return blockByName.find(b) != blockByName.end();
  }
};
