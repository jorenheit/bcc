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
