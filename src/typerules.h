#pragma once
#include "types.h"
#include "common.h"

namespace types::rules {

  struct OpResult {
    TypeHandle type;
    std::string errorMsg;    
    operator bool() const { return type != nullptr; }
  };

  OpResult binOpResult(BinOp op, TypeHandle lhs, TypeHandle rhs);
  OpResult assignResult(TypeHandle dst, TypeHandle src);
  
} // namespace types::rules
  
