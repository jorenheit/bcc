#pragma once
#include <unordered_map>
#include "slot.h"
#include "functionsignature.h"

// ============================================================
// Frame layout
// ============================================================

struct FrameLayout { // TODO: rename to Frame

  enum Offsets {
    TargetBlock,
    RunState,    
    ReturnValueStart
  };

  enum FrameMarkerValue {
    GlobalVariableFrameID = 1,
    FirstStackFrameID = 2
  };

  int returnValueSize = 1;
  //  std::unordered_map<std::string, Slot> locals;
  std::vector<Slot> locals;

  FrameLayout(int retSize): returnValueSize(retSize) {}
  
  inline int localAreaSize() const {
    int result = 0;
    for (auto const &slot: locals) result += slot.size();
    return result;
  }

  inline int localBase() const {
    return ReturnValueStart + returnValueSize;
  }
  
  inline int totalLogicalCells() const {
    return localBase() + localAreaSize();
  }
  
};
