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
    setToValue(1);
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

void Compiler::eqDestructive(Cell other, Temps<1> tmp) {
  auto [cur, oth] = getFieldIndices(_dp.current(), other);
  emit<primitive::Equal>(cur, oth);
}

void Compiler::eqConstructive(Cell result, Cell other, Temps<1> tmp) {
  pushPtr();
  copyField(result, tmp);
  moveTo(result);
  eqDestructive(other, tmp);
  popPtr();
}

void Compiler::eq16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<1> tmp) {
  Cell const currentLow = _dp.current();
  Cell const currentHigh = high;

  pushPtr();
  moveTo(currentLow);
  eqDestructive(otherLow, tmp.select<0>());
  moveTo(currentHigh);
  eqDestructive(otherHigh, tmp.select<0>());
  moveTo(currentLow);
  andDestructive(currentHigh, tmp.select<0>());
  popPtr();
}

void Compiler::eq16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<4> tmp) { 

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
  eq16Destructive(resultHigh, otherCopyLow, otherCopyHigh, tmp.select<3>());
  popPtr();
}


void Compiler::lessDestructive(Cell other, Temps<2> tmp) { 
  auto [cur, oth, tmp0, tmp1] = getFieldIndices(_dp.current(), other, tmp.get<0>(), tmp.get<1>());
  emit<primitive::Less>(cur, oth, tmp0, tmp1);
}

void Compiler::lessConstructive(Cell result, Cell other, Temps<3> tmp) {
  Cell const &otherCopy = tmp.get<0>();
  pushPtr();
  copyField(result, tmp.select<1>());
  moveTo(other);
  copyField(otherCopy, tmp.select<1>());
  moveTo(result);
  lessDestructive(otherCopy, tmp.select<1, 2>());
  popPtr();
}

void Compiler::less16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<4> tmp) {
  // xH < yH || (xH == yH && xL < yL)

  pushPtr();
  
  Cell const currentLow = _dp.current();
  Cell const currentHigh = high;
  Cell const currentCopyHigh = tmp.get<0>();
  Cell const otherCopyHigh = tmp.get<1>();

  moveTo(otherHigh);   copyField(otherCopyHigh, tmp.select<2>());
  moveTo(currentHigh); copyField(currentCopyHigh, tmp.select<2>());
  
  moveTo(currentHigh);
  lessDestructive(otherHigh, tmp.select<2, 3>());
  Cell const highByteLess = currentHigh;
  // otherHigh cleared
  // currentHigh: xH < yH

  moveTo(currentCopyHigh);
  eqDestructive(otherCopyHigh, tmp.select<2>());
  Cell const highByteEqual = currentCopyHigh;
  // otherCopyHigh cleared
  // currentCopyHigh: xH == yH
  
  moveTo(currentLow);
  lessDestructive(otherLow, tmp.select<2, 3>()); 
  Cell const lowByteLess = currentLow;
  // otherLow cleared
  // currentLow: xL < yL

  moveTo(highByteEqual); 
  andDestructive(lowByteLess, tmp.select<2>()); 
  // currentLow cleared
  // currentCopyHigh: xH == yH && xL < yL
  
  orDestructive(highByteLess, tmp.select<2>());
  // currentHigh cleared
  // currentCopyHigh: xH < yH || (xH == yH && xL < yL)
  
  moveField(currentLow);
  // currentCopyHigh cleared
  // currentLow contains result

  popPtr();
  
}

void Compiler::less16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<8> tmp) {

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
  less16Destructive(resultHigh, otherCopyLow, otherCopyHigh, tmp.select<4, 5, 6, 7>());
  popPtr();  
}


void Compiler::lessOrEqualDestructive(Cell other, Temps<2> tmp) {
  auto [cur, oth, tmp0, tmp1] = getFieldIndices(_dp.current(), other, tmp.get<0>(), tmp.get<1>());
  emit<primitive::LessOrEqual>(cur, oth, tmp0, tmp1);
}

void Compiler::lessOrEqualConstructive(Cell result, Cell other, Temps<3> tmp) {
  Cell const &otherCopy = tmp.get<0>();
  pushPtr();
  copyField(result, tmp.select<1>());
  moveTo(other);
  copyField(otherCopy, tmp.select<1>());
  moveTo(result);
  lessOrEqualDestructive(otherCopy, tmp.select<1, 2>());
  popPtr();
}

void Compiler::lessOrEqual16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<4> tmp) {
  greater16Destructive(high, otherLow, otherHigh, tmp);
  notDestructive(tmp.select<0>());
}

void Compiler::lessOrEqual16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<8> tmp) {

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
  lessOrEqual16Destructive(resultHigh, otherCopyLow, otherCopyHigh, tmp.select<4, 5, 6, 7>());
  popPtr();  

}

void Compiler::greaterDestructive(Cell other, Temps<2> tmp) {
  auto [cur, oth, tmp0, tmp1] = getFieldIndices(_dp.current(), other, tmp.get<0>(), tmp.get<1>());
  emit<primitive::Greater>(cur, oth, tmp0, tmp1);
}

void Compiler::greaterConstructive(Cell result, Cell other, Temps<3> tmp) {
  Cell const &otherCopy = tmp.get<0>();
  pushPtr();
  copyField(result, tmp.select<1>());
  moveTo(other);
  copyField(otherCopy, tmp.select<1>());
  moveTo(result);
  greaterDestructive(otherCopy, tmp.select<1, 2>());
  popPtr();
}


void Compiler::greater16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<4> tmp) {
  // xH > yH || (xH == yH && xL > yL)

  pushPtr();
  
  Cell const currentLow = _dp.current();
  Cell const currentHigh = high;
  Cell const currentCopyHigh = tmp.get<0>();
  Cell const otherCopyHigh = tmp.get<1>();

  moveTo(otherHigh);   copyField(otherCopyHigh, tmp.select<2>());
  moveTo(currentHigh); copyField(currentCopyHigh, tmp.select<2>());
  
  moveTo(currentHigh);
  greaterDestructive(otherHigh, tmp.select<2, 3>());
  Cell const highByteGreater = currentHigh;
  // otherHigh cleared
  // currentHigh: xH > yH
  
  moveTo(currentCopyHigh);
  eqDestructive(otherCopyHigh, tmp.select<2>());
  Cell const highByteEqual = currentCopyHigh;
  // otherCopyHigh cleared
  // currentCopyHigh: xH == yH
  
  moveTo(currentLow);
  greaterDestructive(otherLow, tmp.select<2, 3>()); 
  Cell const lowByteGreater = currentLow;
  // otherLow cleared
  // currentLow: xL > yL

  moveTo(highByteEqual); 
  andDestructive(lowByteGreater, tmp.select<2>()); 
  // currentLow cleared
  // currentCopyHigh: xH == yH && xL > yL
  
  orDestructive(highByteGreater, tmp.select<2>());
  // currentHigh cleared
  // currentCopyHigh: xH > yH || (xH == yH && xL > yL)
  
  moveField(currentLow);
  // currentCopyHigh cleared
  // currentLow contains result

  popPtr();
  
}

void Compiler::greater16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<8> tmp) {

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
  greater16Destructive(resultHigh, otherCopyLow, otherCopyHigh, tmp.select<4, 5, 6, 7>());
  popPtr();  
}



void Compiler::greaterOrEqualDestructive(Cell other, Temps<2> tmp) {
  auto [cur, oth, tmp0, tmp1] = getFieldIndices(_dp.current(), other, tmp.get<0>(), tmp.get<1>());
  emit<primitive::GreaterOrEqual>(cur, oth, tmp0, tmp1);
}

void Compiler::greaterOrEqualConstructive(Cell result, Cell other, Temps<3> tmp) {
  Cell const &otherCopy = tmp.get<0>();
  pushPtr();
  copyField(result, tmp.select<1>());
  moveTo(other);
  copyField(otherCopy, tmp.select<1>());
  moveTo(result);
  greaterOrEqualDestructive(otherCopy, tmp.select<1, 2>());
  popPtr();
}

void Compiler::greaterOrEqual16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<4> tmp) {
  less16Destructive(high, otherLow, otherHigh, tmp);
  notDestructive(tmp.select<0>());
}

void Compiler::greaterOrEqual16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<8> tmp) {

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
  greaterOrEqual16Destructive(resultHigh, otherCopyLow, otherCopyHigh, tmp.select<4, 5, 6, 7>());
  popPtr();  

}
