#include "compiler.ih"

Compiler::Cop const Compiler::eqSpec {
  .op = BinOp::Eq,
  .fold = [](int x, int y) -> bool { return x == y; },
  .applyWithSlot = &Compiler::slotEqualSlot,
  .applyWithConst = &Compiler::slotEqualConst
};

Compiler::Cop const Compiler::neqSpec {
  .op = BinOp::Neq,
  .fold = [](int x, int y) -> bool { return x != y; },
  .applyWithSlot = &Compiler::slotNotEqualSlot,
  .applyWithConst = &Compiler::slotNotEqualConst
};

Compiler::Cop const Compiler::ltSpec {
  .op = BinOp::Lt,
  .fold = [](int x, int y) -> bool { return x < y; },
  .applyWithSlot = &Compiler::slotLessSlot,
  .applyWithConst = &Compiler::slotLessConst
};

Compiler::Cop const Compiler::leSpec {
  .op = BinOp::Le,
  .fold = [](int x, int y) -> bool { return x <= y; },
  .applyWithSlot = &Compiler::slotLessEqualSlot,
  .applyWithConst = &Compiler::slotLessEqualConst
};

Compiler::Cop const Compiler::gtSpec {
  .op = BinOp::Gt,
  .fold = [](int x, int y) -> bool { return x > y; },
  .applyWithSlot = &Compiler::slotGreaterSlot,
  .applyWithConst = &Compiler::slotGreaterConst
};

Compiler::Cop const Compiler::geSpec {
  .op = BinOp::Ge,
  .fold = [](int x, int y) -> bool { return x >= y; },
  .applyWithSlot = &Compiler::slotGreaterEqualSlot,
  .applyWithConst = &Compiler::slotGreaterEqualConst
};


void Compiler::slotEqualConst(Slot const &lhs, int val) { assert(false); }
void Compiler::slotEqualSlot(Slot const &lhs, Slot const &rhs) { assert(false); }
void Compiler::slotNotEqualConst(Slot const &lhs, int val) { assert(false); }
void Compiler::slotNotEqualSlot(Slot const &lhs, Slot const &rhs) { assert(false); }
void Compiler::slotLessConst(Slot const &lhs, int val) { assert(false); }
void Compiler::slotLessSlot(Slot const &lhs, Slot const &rhs) { assert(false); }
void Compiler::slotLessEqualConst(Slot const &lhs, int val) { assert(false); }
void Compiler::slotLessEqualSlot(Slot const &lhs, Slot const &rhs) { assert(false); }
void Compiler::slotGreaterConst(Slot const &lhs, int val) { assert(false); }
void Compiler::slotGreaterSlot(Slot const &lhs, Slot const &rhs) { assert(false); }
void Compiler::slotGreaterEqualConst(Slot const &lhs, int val) { assert(false); }
void Compiler::slotGreaterEqualSlot(Slot const &lhs, Slot const &rhs) { assert(false); }
