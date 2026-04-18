#pragma once

// TODO: where to put this stuff?

namespace types {
  struct Type;  
};

struct StructField {
  std::string name;
  types::Type const *type;
};

using StructFields = std::vector<StructField>;

enum class BinOp {
  Add, Sub, Mul, Div, Mod, And, Or, Xor
};

inline std::string binOpStr(BinOp op) {
  switch (op) {
  case BinOp::Add: return "+";
  case BinOp::Sub: return "-";
  case BinOp::Mul: return "*";
  case BinOp::Div: return "/";
  case BinOp::Mod: return "%";
  case BinOp::And: return "&&";
  case BinOp::Or:  return "||";
  case BinOp::Xor: return "^";
  default: std::unreachable();
  }
}

  
