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

  if (lhs.type->usesValue1() || rhs.type->usesValue1()) {
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

  if (lhs.type->usesValue1() || rhs.type->usesValue1()) {
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
  pushPtr();
  moveTo(lhs);

  if (val > 0) {
    setToValue(1);
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

void Compiler::orSlotWithSlot(Slot const &lhs, Slot const &rhs) {
  pushPtr();

  Slot const rhsCopy = getTemp(rhs.type);
  assignSlot(rhsCopy, rhs);
  moveTo(lhs);

  if (lhs.type->usesValue1() || rhs.type->usesValue1()) {
    or16Destructive(Cell{lhs, MacroCell::Value1},
		    Cell{rhsCopy, MacroCell::Value0},
		    Cell{rhsCopy, MacroCell::Value1},
		    Temps<1>::select(lhs, MacroCell::Scratch0));
		     
  } else {
    orDestructive(Cell{rhsCopy, MacroCell::Value0},
		  Temps<1>::select(lhs, MacroCell::Scratch0));
  }

  popPtr();
}

void Compiler::norSlotWithConst(Slot const &lhs, int val) {
  pushPtr();
  moveTo(lhs);

  if (val > 0) {
    setToValue(0);
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

void Compiler::norSlotWithSlot(Slot const &lhs, Slot const &rhs) {
  pushPtr();

  Slot const rhsCopy = getTemp(rhs.type);
  assignSlot(rhsCopy, rhs);
  moveTo(lhs);

  if (lhs.type->usesValue1() || rhs.type->usesValue1()) {
    nor16Destructive(Cell{lhs, MacroCell::Value1},
		     Cell{rhsCopy, MacroCell::Value0},
		     Cell{rhsCopy, MacroCell::Value1},
		     Temps<1>::select(lhs, MacroCell::Scratch0));
		     
  } else {
    norDestructive(Cell{rhsCopy, MacroCell::Value0},
		   Temps<1>::select(lhs, MacroCell::Scratch0));
  }

  popPtr();
}

void Compiler::xorSlotWithConst(Slot const &lhs, int val) {
  pushPtr();
  moveTo(lhs);

  if (val > 0) {
    if (lhs.type->usesValue1()) {
      not16Destructive(Cell{lhs, MacroCell::Value1},
		       Temps<1>::select(lhs, MacroCell::Scratch0));
    } else {
      notDestructive(Temps<1>::select(lhs, MacroCell::Scratch0));
    }
  }
  else {
    if (lhs.type->usesValue1()) {
      bool16Destructive(Cell{lhs, MacroCell::Value1},
		       Temps<1>::select(lhs, MacroCell::Scratch0));
    } else {
      boolDestructive(Temps<1>::select(lhs, MacroCell::Scratch0));
    }
  }

  popPtr();
}

void Compiler::xorSlotWithSlot(Slot const &lhs, Slot const &rhs) {
  pushPtr();

  Slot const rhsCopy = getTemp(rhs.type);
  assignSlot(rhsCopy, rhs);
  moveTo(lhs);

  if (lhs.type->usesValue1() || rhs.type->usesValue1()) {
    xor16Destructive(Cell{lhs, MacroCell::Value1},
		     Cell{rhsCopy, MacroCell::Value0},
		     Cell{rhsCopy, MacroCell::Value1},
		     Temps<2>::select(lhs, MacroCell::Scratch0,
				      lhs, MacroCell::Scratch1));
		     
  } else {
    xorDestructive(Cell{rhsCopy, MacroCell::Value0},
		   Temps<2>::select(lhs, MacroCell::Scratch0,
				    lhs, MacroCell::Scratch1));
  }

  popPtr();
}

void Compiler::xnorSlotWithConst(Slot const &lhs, int val) {
  pushPtr();
  moveTo(lhs);

  if (val == 0) {
    if (lhs.type->usesValue1()) {
      not16Destructive(Cell{lhs, MacroCell::Value1},
		       Temps<1>::select(lhs, MacroCell::Scratch0));
    } else {
      notDestructive(Temps<1>::select(lhs, MacroCell::Scratch0));
    }
  }
  else {
    if (lhs.type->usesValue1()) {
      bool16Destructive(Cell{lhs, MacroCell::Value1},
		       Temps<1>::select(lhs, MacroCell::Scratch0));
    } else {
      boolDestructive(Temps<1>::select(lhs, MacroCell::Scratch0));
    }
  }

  popPtr();
}

void Compiler::xnorSlotWithSlot(Slot const &lhs, Slot const &rhs) {
  pushPtr();

  Slot const rhsCopy = getTemp(rhs.type);
  assignSlot(rhsCopy, rhs);
  moveTo(lhs);

  if (lhs.type->usesValue1() || rhs.type->usesValue1()) {
    xnor16Destructive(Cell{lhs, MacroCell::Value1},
		      Cell{rhsCopy, MacroCell::Value0},
		      Cell{rhsCopy, MacroCell::Value1},
		      Temps<2>::select(lhs, MacroCell::Scratch0,
				       lhs, MacroCell::Scratch1));
		     
  } else {
    xnorDestructive(Cell{rhsCopy, MacroCell::Value0},
		    Temps<2>::select(lhs, MacroCell::Scratch0,
				     lhs, MacroCell::Scratch1));
  }

  popPtr();
}

void Compiler::orDestructive(Cell other, Temps<1> tmp) {
  auto [cur, oth, tmp0] = getFieldIndices(_dp.current(), other, tmp.get<0>());
  emit<primitive::Or>(cur, oth, tmp0);
}

void Compiler::orConstructive(Cell result, Cell other, Temps<2> tmp) {
  Cell const &otherCopy = tmp.get<0>();
  
  pushPtr();
  copyField(result, tmp.select<1>());  
  moveTo(other);
  copyField(otherCopy, tmp.select<1>());
  moveTo(result);
  orDestructive(otherCopy, tmp.select<1>());
  popPtr();
}

void Compiler::or16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<1> tmp) {

  pushPtr();
  Cell const currentLow = _dp.current();
  Cell const currentHigh = high;

  // Collapse both to bool
  moveTo(currentLow);
  bool16Destructive(currentHigh, tmp);
  moveTo(otherLow);
  bool16Destructive(otherHigh, tmp);

  // And results
  moveTo(currentLow);
  orDestructive(otherLow, tmp);
  popPtr();
}

void Compiler::or16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<4> tmp) { 

  Cell const currentLow = _dp.current();
  Cell const currentHigh = high;
  Cell const otherCopyLow = tmp.get<0>();
  Cell const otherCopyHigh = tmp.get<1>();
  Cell const resultHigh = tmp.get<2>();
  
  pushPtr();

  moveTo(currentLow);  copyField(result, tmp.select<3>());
  moveTo(currentHigh); copyField(resultHigh, tmp.select<3>());
  moveTo(otherLow);    copyField(otherCopyLow, tmp.select<3>());
  moveTo(otherHigh);   copyField(otherCopyHigh, tmp.select<3>());

  moveTo(result);
  or16Destructive(resultHigh, otherCopyLow, otherCopyHigh, tmp.select<3>());
  popPtr();
}

void Compiler::andDestructive(Cell other, Temps<1> tmp) {
  auto [cur, oth, tmp0] = getFieldIndices(_dp.current(), other, tmp.get<0>());
  emit<primitive::And>(cur, oth, tmp0);
}

void Compiler::andConstructive(Cell result, Cell other, Temps<2> tmp) { 
  Cell const &otherCopy = tmp.get<0>();
  pushPtr();
  copyField(result, tmp.select<1>());  
  moveTo(other);
  copyField(otherCopy, tmp.select<1>());
  moveTo(result);
  andDestructive(otherCopy, tmp.select<1>());
  popPtr();
}

void Compiler::and16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<1> tmp) {

  pushPtr();
  Cell const currentLow = _dp.current();
  Cell const currentHigh = high;

  // Collapse both to bool
  moveTo(currentLow);
  bool16Destructive(currentHigh, tmp);
  moveTo(otherLow);
  bool16Destructive(otherHigh, tmp);

  // And results
  moveTo(currentLow);
  andDestructive(otherLow, tmp);
  popPtr();
}


void Compiler::and16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<4> tmp) { 

  Cell const currentLow = _dp.current();
  Cell const currentHigh = high;
  Cell const otherCopyLow = tmp.get<0>();
  Cell const otherCopyHigh = tmp.get<1>();
  Cell const resultHigh = tmp.get<2>();
  
  pushPtr();

  moveTo(currentLow);  copyField(result, tmp.select<3>());
  moveTo(currentHigh); copyField(resultHigh, tmp.select<3>());
  moveTo(otherLow);    copyField(otherCopyLow, tmp.select<3>());
  moveTo(otherHigh);   copyField(otherCopyHigh, tmp.select<3>());

  moveTo(result);
  and16Destructive(resultHigh, otherCopyLow, otherCopyHigh, tmp.select<3>());
  popPtr();
}

void Compiler::xorDestructive(Cell other, Temps<2> tmp) {
  auto [cur, oth, tmp0, tmp1] = getFieldIndices(_dp.current(), other, tmp.get<0>(), tmp.get<1>());
  emit<primitive::Xor>(cur, oth, tmp0, tmp1);
}

void Compiler::xorConstructive(Cell result, Cell other, Temps<3> tmp) { 
  Cell const &otherCopy = tmp.get<0>();
  pushPtr();
  copyField(result, tmp.select<1>());  
  moveTo(other);
  copyField(otherCopy, tmp.select<1>());
  moveTo(result);
  xorDestructive(otherCopy, tmp.select<1, 2>());
  popPtr();
}

void Compiler::xor16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<2> tmp) {
  pushPtr();
  Cell const currentLow = _dp.current();
  Cell const currentHigh = high;

  // Collapse both to bool
  moveTo(currentLow);
  bool16Destructive(currentHigh, tmp.select<0>());
  moveTo(otherLow);
  bool16Destructive(otherHigh, tmp.select<0>());

  // And results
  moveTo(currentLow);
  xorDestructive(otherLow, tmp);
  popPtr();
}


void Compiler::xor16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<5> tmp) { 

  Cell const currentLow = _dp.current();
  Cell const currentHigh = high;
  Cell const otherCopyLow = tmp.get<0>();
  Cell const otherCopyHigh = tmp.get<1>();
  Cell const resultHigh = tmp.get<2>();
  
  pushPtr();

  moveTo(currentLow);  copyField(result, tmp.select<3>());
  moveTo(currentHigh); copyField(resultHigh, tmp.select<3>());
  moveTo(otherLow);    copyField(otherCopyLow, tmp.select<3>());
  moveTo(otherHigh);   copyField(otherCopyHigh, tmp.select<3>());

  moveTo(result);
  xor16Destructive(resultHigh, otherCopyLow, otherCopyHigh, tmp.select<3, 4>());
  popPtr();
}

void Compiler::nandDestructive(Cell other, Temps<1> tmp) {
  andDestructive(other, tmp);
  notDestructive(tmp);
}

void Compiler::nandConstructive(Cell result, Cell other, Temps<2> tmp) {
  andConstructive(result, other, tmp);
  notDestructive(tmp.select<0>());
}

void Compiler::nand16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<1> tmp) {
  and16Destructive(high, otherLow, otherHigh, tmp);
  notDestructive(tmp);
}


void Compiler::nand16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<4> tmp) {
  and16Constructive(high, result, otherLow, otherHigh, tmp);
  notDestructive(tmp.select<0>());
}

void Compiler::norDestructive(Cell other, Temps<1> tmp) {
  orDestructive(other, tmp);
  notDestructive(tmp);
}

void Compiler::norConstructive(Cell result, Cell other, Temps<2> tmp) {
  orConstructive(result, other, tmp);
  notDestructive(tmp.select<0>());
}

void Compiler::nor16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<1> tmp) {
  or16Destructive(high, otherLow, otherHigh, tmp);
  notDestructive(tmp);
}

void Compiler::nor16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<4> tmp) {
  or16Constructive(high, result, otherLow, otherHigh, tmp);
  notDestructive(tmp.select<0>());
}

void Compiler::xnorDestructive(Cell other, Temps<2> tmp) {
  xorDestructive(other, tmp);
  notDestructive(tmp.select<1>());
}

void Compiler::xnorConstructive(Cell result, Cell other, Temps<3> tmp) {
  xorConstructive(result, other, tmp);
  notDestructive(tmp.select<0>());
}

void Compiler::xnor16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<2> tmp) {
  xor16Destructive(high, otherLow, otherHigh, tmp);
  notDestructive(tmp.select<0>());
}

void Compiler::xnor16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<5> tmp) {
  xor16Constructive(high, result, otherLow, otherHigh, tmp);
  notDestructive(tmp.select<0>());
}
