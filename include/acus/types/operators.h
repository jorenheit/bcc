#pragma once

#include <string>

namespace acus {

  enum class BinOp {
    Add, Sub, Mul, Div, Mod,
    And, Or, Nand, Nor, Xor, Xnor,
    Eq, Neq, Lt, Le, Gt, Ge
  };

  enum class UnOp {
    Not, Bool, Neg, Abs, Sign
  };
  
  std::string binOpStr(BinOp op);
  std::string unOpStr(UnOp op);

} // namespace acus
