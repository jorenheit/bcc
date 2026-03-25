#pragma once

#include <variant>
#include <vector>
#include <unordered_map>
#include <memory>

#include "slot.h"
#include "data.h"
#include "primitive.h"
#include "frame.h"

// ============================================================
// Function
// ============================================================


struct Function {

  class Argument {
    std::variant<Slot, values::Value> arg;
  public:
    Argument(Slot const &s): arg(s) {}
    Argument(values::Value const &v): arg(v) {}

    bool hasSlot() const { return std::holds_alternative<Slot>(arg); }
    Slot getSlot() const {
      assert(hasSlot());
      return std::get<Slot>(arg);
    }

    bool hasValue() const { return std::holds_alternative<values::Value>(arg); }    
    values::Value getValue() const {
      assert(hasValue());
      return std::get<values::Value>(arg);
    }
  };
  
  struct Block {
    size_t globalBlockIndex = 0;
    size_t parentFunctionIndex = 0;
    std::string name;
    std::string id;
    primitive::Sequence code;
  };

  struct Scope {
    Scope *parent = nullptr;
  };
  
  size_t functionIndex = 0;
  std::string name;
  FrameLayout frame;
  FunctionSignature sig;
  
  std::vector<std::unique_ptr<Block>> blocks;
  std::unordered_map<std::string, size_t> blockByName;
  std::vector<std::unique_ptr<Scope>> scopes;
  
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

  inline Scope &createScope(Scope *parent) {
    bool valid = false;
    for (auto const &scope: scopes) {
      if (scope.get() == parent) {
	valid = true;
	break;
      }
    }
    assert(valid || parent == nullptr);
    scopes.emplace_back(std::make_unique<Scope>(parent));
    return *scopes.back();
  }
};
