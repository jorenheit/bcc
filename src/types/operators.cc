// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <utility>

#include "acus/types/operators.h"

std::string acus::binOpStr(BinOp op) {
  switch (op) {
  case BinOp::Add:  return "+";
  case BinOp::Sub:  return "-";
  case BinOp::Mul:  return "*";
  case BinOp::Div:  return "/";
  case BinOp::Mod:  return "%";
  case BinOp::And:  return "&";
  case BinOp::Nand: return "~&";
  case BinOp::Or:   return "|";
  case BinOp::Nor:  return "~|";
  case BinOp::Xor:  return "^";
  case BinOp::Xnor: return "~^";
  case BinOp::Eq:   return "==";
  case BinOp::Neq:  return "!=";
  case BinOp::Lt:   return "<";
  case BinOp::Le:   return "<=";
  case BinOp::Gt:   return ">";
  case BinOp::Ge:   return ">=";
  }
  std::unreachable();
}

std::string acus::unOpStr(UnOp op) {
  switch (op) {
  case UnOp::Not:     return "!";
  case UnOp::Bool:    return "!!";
  case UnOp::Neg:     return "-";
  case UnOp::Abs:     return "abs ";
  case UnOp::SignBit: return "sgn ";
  }
  std::unreachable();
}
