#pragma once
#include <unordered_map>
#include "slot.h"

// ============================================================
// Frame layout
// ============================================================

struct FrameLayout {

  enum Offsets {
    TargetBlockOffset,
    RunStateOffset,    
    ReturnValueOffset, 
    LocalBaseOffset   
  };

  std::unordered_map<std::string, Slot> locals;

  inline int localAreaSize() const {
    int result = 0;
    for (auto const &[_, slot]: locals) result += slot.size;
    return result;
  }

  inline int totalLogicalCells() const {
    return LocalBaseOffset + localAreaSize();
  }
  
};
