#pragma once
#include <string>
#include "bfc/types/types_fwd.h"
#include "bfc/types/operators.h"

namespace types::rules {

  struct OpResult {
    TypeHandle type;
    std::string errorMsg;    
    operator bool() const { return type != nullptr; }
  };

  OpResult binOpResult(BinOp op, TypeHandle lhs, TypeHandle rhs);
  OpResult assignResult(TypeHandle dst, TypeHandle src);
  
} // namespace types::rules
