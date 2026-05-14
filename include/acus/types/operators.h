// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

namespace acus {

  enum class BinOp {
    Add, Sub, Mul, Div, Mod,
    And, Or, Nand, Nor, Xor, Xnor,
    Eq, Neq, Lt, Le, Gt, Ge
  };

  enum class UnOp {
    Not, Bool, Neg, Abs, SignBit
  };
  
  std::string binOpStr(BinOp op);
  std::string unOpStr(UnOp op);

} // namespace acus
