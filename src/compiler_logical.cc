#include "compiler.ih"

Compiler::Lop const Compiler::landSpec {
  .op = BinOp::And,
  .fold = [](bool x, bool y) -> bool { return x && y; },
  .applyWithSlot = &Compiler::andSlotWithSlot,
  .applyWithConst = &Compiler::andSlotWithConst
};

Compiler::Lop const Compiler::lnandSpec {
  .op = BinOp::Nand,
  .fold = [](bool x, bool y) -> bool { return !(x && y); },
  .applyWithSlot = &Compiler::nandSlotWithSlot,
  .applyWithConst = &Compiler::nandSlotWithConst
};

Compiler::Lop const Compiler::lorSpec {
  .op = BinOp::Or,
  .fold = [](bool x, bool y) -> bool { return x || y; },
  .applyWithSlot = &Compiler::orSlotWithSlot,
  .applyWithConst = &Compiler::orSlotWithConst
};

Compiler::Lop const Compiler::lnorSpec {
  .op = BinOp::Nor,
  .fold = [](bool x, bool y) -> bool { return !(x || y); },
  .applyWithSlot = &Compiler::norSlotWithSlot,
  .applyWithConst = &Compiler::norSlotWithConst
};

Compiler::Lop const Compiler::lxorSpec {
  .op = BinOp::Xor,
  .fold = [](bool x, bool y) -> bool { return x != y; },
  .applyWithSlot = &Compiler::xorSlotWithSlot,
  .applyWithConst = &Compiler::xorSlotWithConst
};

Compiler::Lop const Compiler::lxnorSpec {
  .op = BinOp::Xnor,
  .fold = [](bool x, bool y) -> bool { return x == y; },
  .applyWithSlot = &Compiler::xnorSlotWithSlot,
  .applyWithConst = &Compiler::xnorSlotWithConst
};

void Compiler::andSlotWithConst(Slot const &lhs, int val) {
  pushPtr();
  moveTo(lhs);

  if (val == 0) {
    zeroCell();
    moveTo(lhs, MacroCell::Value1);
    zeroCell();
    popPtr();
    return;
  }

  if (lhs.type->usesValue1()) {
    bool16Destructive(Cell{lhs, MacroCell::Value1},
		      Temps<1>::select(lhs, MacroCell::Scratch0));
  } else {
    boolDestructive(Temps<1>::select(lhs, MacroCell::Scratch0));
  }
  
  popPtr();
}

void Compiler::andSlotWithSlot(Slot const &lhs, Slot const &rhs) {
  pushPtr();

  Slot const rhsCopy = getTemp(rhs.type);
  assignSlot(rhsCopy, rhs);
  moveTo(lhs);

  if (lhs.type->usesValue1()) {
    and16Destructive(Cell{lhs, MacroCell::Value1},
		     Cell{rhsCopy, MacroCell::Value0},
		     Cell{rhsCopy, MacroCell::Value1},
		     Temps<1>::select(lhs, MacroCell::Scratch0));
		     
  } else {
    andDestructive(Cell{rhsCopy, MacroCell::Value0},
		   Temps<1>::select(lhs, MacroCell::Scratch0));
  }

  popPtr();
}


void Compiler::nandSlotWithConst(Slot const &lhs, int val) {
  pushPtr();
  moveTo(lhs);

  if (val == 0) {
    setToValue(1);
    moveTo(lhs, MacroCell::Value1);
    zeroCell();
    popPtr();
    return;
  }

  if (lhs.type->usesValue1()) {
    not16Destructive(Cell{lhs, MacroCell::Value1},
		     Temps<1>::select(lhs, MacroCell::Scratch0));
  } else {
    notDestructive(Temps<1>::select(lhs, MacroCell::Scratch0));
  }
  
  popPtr();
}

void Compiler::nandSlotWithSlot(Slot const &lhs, Slot const &rhs) {
  pushPtr();

  Slot const rhsCopy = getTemp(rhs.type);
  assignSlot(rhsCopy, rhs);
  moveTo(lhs);

  if (lhs.type->usesValue1()) {
    nand16Destructive(Cell{lhs, MacroCell::Value1},
		      Cell{rhsCopy, MacroCell::Value0},
		      Cell{rhsCopy, MacroCell::Value1},
		      Temps<1>::select(lhs, MacroCell::Scratch0));
		     
  } else {
    nandDestructive(Cell{rhsCopy, MacroCell::Value0},
		    Temps<1>::select(lhs, MacroCell::Scratch0));
  }

  popPtr();
}

void Compiler::orSlotWithConst(Slot const &lhs, int val) {
  assert(false);
}

void Compiler::orSlotWithSlot(Slot const &lhs, Slot const &rhs) {
  assert(false);
}

void Compiler::norSlotWithConst(Slot const &lhs, int val) {
  assert(false);
}

void Compiler::norSlotWithSlot(Slot const &lhs, Slot const &rhs) {
  assert(false);
}

void Compiler::xorSlotWithConst(Slot const &lhs, int val) {
  assert(false);
}

void Compiler::xorSlotWithSlot(Slot const &lhs, Slot const &rhs) {
  assert(false);
}

void Compiler::xnorSlotWithConst(Slot const &lhs, int val) {
  assert(false);
}

void Compiler::xnorSlotWithSlot(Slot const &lhs, Slot const &rhs) {
  assert(false);
}
