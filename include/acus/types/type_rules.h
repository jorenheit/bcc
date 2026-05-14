// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
