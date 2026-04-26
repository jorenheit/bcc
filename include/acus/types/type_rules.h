#pragma once
#include <string>
#include "acus/types/types_fwd.h"
#include "acus/types/operators.h"

namespace acus::types::rules {

  struct OpResult {
    TypeHandle type;
    std::string errorMsg;    
    operator bool() const { return type != nullptr; }
  };

  OpResult binOpResult(BinOp op, TypeHandle lhs, TypeHandle rhs);
  OpResult assignResult(TypeHandle dst, TypeHandle src);
  
} // namespace acus::types::rules
