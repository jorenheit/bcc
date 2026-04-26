#pragma once

#include <string>

namespace acus {

enum class BinOp {
  Add, Sub, Mul, Div, Mod,
  And, Or, Nand, Nor, Xor, Xnor,
  Eq, Neq, Lt, Le, Gt, Ge
};

std::string binOpStr(BinOp op);

} // namespace acus