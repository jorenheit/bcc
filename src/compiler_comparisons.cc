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

void Compiler::slotEqualConst(Slot const &lhs, int val) {
  pushPtr();

  moveTo(lhs);  
  subConstFromSlot(lhs, val);
  
  if (lhs.type->usesValue1()) {
    not16Destructive(Cell{lhs, MacroCell::Value1},
		     Temps<1>::select(lhs, MacroCell::Scratch0));
  } else {
    notDestructive(Temps<1>::select(lhs, MacroCell::Scratch0));
  }

  popPtr();
}

void Compiler::slotEqualSlot(Slot const &lhs, Slot const &rhs) {
  pushPtr();
  Slot const rhsCopy = getTemp(rhs.type);
  assignSlot(rhsCopy, rhs);
  moveTo(lhs);

  if (lhs.type->usesValue1() || rhs.type->usesValue1()) {
    eq16Destructive(Cell{lhs, MacroCell::Value1},
		    Cell{rhsCopy, MacroCell::Value0},
		    Cell{rhsCopy, MacroCell::Value1},
		    Temps<1>::select(lhs, MacroCell::Scratch0));
  } else {
    eqDestructive(Cell{rhsCopy, MacroCell::Value0},
		  Temps<1>::select(lhs, MacroCell::Scratch0));
  }

  popPtr();
}

void Compiler::slotNotEqualConst(Slot const &lhs, int val) {
  pushPtr();
  slotEqualConst(lhs, val);
  moveTo(lhs);
  notDestructive(Temps<1>::select(lhs, MacroCell::Scratch0));
  popPtr();
}

void Compiler::slotNotEqualSlot(Slot const &lhs, Slot const &rhs) {
  pushPtr();
  Slot const rhsCopy = getTemp(rhs.type);
  assignSlot(rhsCopy, rhs);
  slotEqualSlot(lhs, rhsCopy);
  moveTo(lhs);
  notDestructive(Temps<1>::select(lhs, MacroCell::Scratch0));
  popPtr();
}

void Compiler::slotLessConst(Slot const &lhs, int val) {
  if (val == 0) {
    pushPtr();
    moveTo(lhs);
    setToValue16(0, Cell{lhs, MacroCell::Value1});
    popPtr();
    return;
  }

  pushPtr();

  Slot const valSlot = getTemp(((val >> 8) & 0xff) ? values::i16(val) : values::i8(val));
  slotLessSlot(lhs, valSlot);

  popPtr();
}

void Compiler::slotLessSlot(Slot const &lhs, Slot const &rhs) {
  
  pushPtr();

  Slot const rhsCopy = getTemp(rhs.type);
  assignSlot(rhsCopy, rhs);

  moveTo(lhs);  
  if (lhs.type->usesValue1() || rhs.type->usesValue1()) {
    less16Destructive(Cell{lhs, MacroCell::Value1},
		      Cell{rhsCopy, MacroCell::Value0},
		      Cell{rhsCopy, MacroCell::Value1},
		      Temps<4>::select(lhs, MacroCell::Scratch0,
				       lhs, MacroCell::Scratch1,
				       rhsCopy, MacroCell::Scratch0,
				       rhsCopy, MacroCell::Scratch1));
  } else {
    lessDestructive(Cell{rhsCopy, MacroCell::Value0},
		    Temps<2>::select(lhs, MacroCell::Scratch0,
				     lhs, MacroCell::Scratch1));
  }

  popPtr();
}

void Compiler::slotLessEqualConst(Slot const &lhs, int val) {
  pushPtr();

  // If val is maximal, the result must be true
  if (lhs.type->usesValue1() && ((val & 0xffff) == 0xffff)) {
    moveTo(lhs); 
    setToValue16(1, Cell{lhs, MacroCell::Value1});
    popPtr();
  }
  else if ((val & 0xff) == 0xff) {
    moveTo(lhs);
    setToValue(1);
    popPtr();
  }
  
  Slot const valSlot = getTemp(((val >> 8) & 0xff) ? values::i16(val) : values::i8(val));
  slotLessEqualSlot(lhs, valSlot);

  popPtr();
  
}

void Compiler::slotLessEqualSlot(Slot const &lhs, Slot const &rhs) {

  pushPtr();

  Slot const rhsCopy = getTemp(rhs.type);
  assignSlot(rhsCopy, rhs);

  moveTo(lhs);  
  if (lhs.type->usesValue1() || rhs.type->usesValue1()) {
    lessOrEqual16Destructive(Cell{lhs, MacroCell::Value1},
			     Cell{rhsCopy, MacroCell::Value0},
			     Cell{rhsCopy, MacroCell::Value1},
			     Temps<4>::select(lhs, MacroCell::Scratch0,
					      lhs, MacroCell::Scratch1,
					      rhsCopy, MacroCell::Scratch0,
					      rhsCopy, MacroCell::Scratch1));
  } else {
    lessOrEqualDestructive(Cell{rhsCopy, MacroCell::Value0},
			   Temps<2>::select(lhs, MacroCell::Scratch0,
					    lhs, MacroCell::Scratch1));
  }

  popPtr();
}


void Compiler::slotGreaterConst(Slot const &lhs, int val) {

  pushPtr();

  // If val is maximal, the result must be false
  if (lhs.type->usesValue1() && ((val & 0xffff) == 0xffff)) {
    moveTo(lhs); 
    setToValue16(0, Cell{lhs, MacroCell::Value1});
    popPtr();
    return;
  }
  else if ((val & 0xff) == 0xff) {
    moveTo(lhs);
    setToValue(0);
    popPtr();
    return;
  }

  Slot const valSlot = getTemp(((val >> 8) & 0xff) ? values::i16(val) : values::i8(val));
  slotGreaterSlot(lhs, valSlot);

  popPtr();
}

void Compiler::slotGreaterSlot(Slot const &lhs, Slot const &rhs) {
  
  pushPtr();

  Slot const rhsCopy = getTemp(rhs.type);
  assignSlot(rhsCopy, rhs);

  moveTo(lhs);  
  if (lhs.type->usesValue1() || rhs.type->usesValue1()) {
    greater16Destructive(Cell{lhs, MacroCell::Value1},
			 Cell{rhsCopy, MacroCell::Value0},
			 Cell{rhsCopy, MacroCell::Value1},
			 Temps<4>::select(lhs, MacroCell::Scratch0,
					  lhs, MacroCell::Scratch1,
					  rhsCopy, MacroCell::Scratch0,
					  rhsCopy, MacroCell::Scratch1));
  } else {
    greaterDestructive(Cell{rhsCopy, MacroCell::Value0},
		       Temps<2>::select(lhs, MacroCell::Scratch0,
					lhs, MacroCell::Scratch1));
  }

  popPtr();
}

void Compiler::slotGreaterEqualConst(Slot const &lhs, int val) {

  pushPtr();

  // If val is 0, the result must be true
  if (lhs.type->usesValue1() && (val == 0)) {
    moveTo(lhs); 
    setToValue16(1, Cell{lhs, MacroCell::Value1});
    popPtr();
    return;
  }
  else if (val == 0) {
    moveTo(lhs);
    setToValue(0);
    popPtr();
    return;
  }

  Slot const valSlot = getTemp(((val >> 8) & 0xff) ? values::i16(val) : values::i8(val));
  slotGreaterEqualSlot(lhs, valSlot);

  popPtr();
}

void Compiler::slotGreaterEqualSlot(Slot const &lhs, Slot const &rhs) {
  
  pushPtr();

  Slot const rhsCopy = getTemp(rhs.type);
  assignSlot(rhsCopy, rhs);

  moveTo(lhs);  
  if (lhs.type->usesValue1() || rhs.type->usesValue1()) {
    greaterOrEqual16Destructive(Cell{lhs, MacroCell::Value1},
				Cell{rhsCopy, MacroCell::Value0},
				Cell{rhsCopy, MacroCell::Value1},
				Temps<4>::select(lhs, MacroCell::Scratch0,
						 lhs, MacroCell::Scratch1,
						 rhsCopy, MacroCell::Scratch0,
						 rhsCopy, MacroCell::Scratch1));
  } else {
    greaterOrEqualDestructive(Cell{rhsCopy, MacroCell::Value0},
			      Temps<2>::select(lhs, MacroCell::Scratch0,
					       lhs, MacroCell::Scratch1));
  }

  popPtr();
}
