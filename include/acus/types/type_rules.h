#pragma once
#include <string>
#include "acus/types/types_fwd.h"
#include "acus/types/operators.h"

namespace acus::types::rules {

  struct OpResult {
    TypeHandle type;
    TypeHandle workType;
    std::string errorMsg;    
    operator bool() const { return type != nullptr; }
  };

  OpResult binOpResult(BinOp op, TypeHandle lhs, TypeHandle rhs);
  OpResult assignResult(TypeHandle dst, TypeHandle src);
  OpResult castResult(TypeHandle from, TypeHandle to);
  
} // namespace acus::types::rules
