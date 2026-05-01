#include "assembler.ih"

Assembler::Cop const Assembler::eqSpec {
  .op = BinOp::Eq,
  .fold = [](int x, int y) -> bool { return x == y; },
  .applyWithSlot = &Assembler::slotEqualSlot,
  .applyWithConst = &Assembler::slotEqualConst
};

Assembler::Cop const Assembler::neqSpec {
  .op = BinOp::Neq,
  .fold = [](int x, int y) -> bool { return x != y; },
  .applyWithSlot = &Assembler::slotNotEqualSlot,
  .applyWithConst = &Assembler::slotNotEqualConst
};

Assembler::Cop const Assembler::ltSpec {
  .op = BinOp::Lt,
  .fold = [](int x, int y) -> bool { return x < y; },
  .applyWithSlot = &Assembler::slotLessSlot,
  .applyWithConst = &Assembler::slotLessConst
};

Assembler::Cop const Assembler::leSpec {
  .op = BinOp::Le,
  .fold = [](int x, int y) -> bool { return x <= y; },
  .applyWithSlot = &Assembler::slotLessEqualSlot,
  .applyWithConst = &Assembler::slotLessEqualConst
};

Assembler::Cop const Assembler::gtSpec {
  .op = BinOp::Gt,
  .fold = [](int x, int y) -> bool { return x > y; },
  .applyWithSlot = &Assembler::slotGreaterSlot,
  .applyWithConst = &Assembler::slotGreaterConst
};

Assembler::Cop const Assembler::geSpec {
  .op = BinOp::Ge,
  .fold = [](int x, int y) -> bool { return x >= y; },
  .applyWithSlot = &Assembler::slotGreaterEqualSlot,
  .applyWithConst = &Assembler::slotGreaterEqualConst
};

void Assembler::slotEqualConst(Slot const &lhs, int val) {

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

void Assembler::slotEqualSlot(Slot const &lhs, Slot const &rhs) {
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

void Assembler::slotNotEqualConst(Slot const &lhs, int val) {
  pushPtr();
  slotEqualConst(lhs, val);
  moveTo(lhs);
  notDestructive(Temps<1>::select(lhs, MacroCell::Scratch0));
  popPtr();
}

void Assembler::slotNotEqualSlot(Slot const &lhs, Slot const &rhs) {
  pushPtr();
  Slot const rhsCopy = getTemp(rhs.type);
  assignSlot(rhsCopy, rhs);
  slotEqualSlot(lhs, rhsCopy);
  moveTo(lhs);
  notDestructive(Temps<1>::select(lhs, MacroCell::Scratch0));
  popPtr();
}

void Assembler::slotLessConst(Slot const &lhs, int val) {
  assert(not types::isSignedInteger(lhs.type) && "not implemented yet");
  
  if (val == 0) {
    pushPtr();
    moveTo(lhs);
    setToValue16(0, Cell{lhs, MacroCell::Value1});
    popPtr();
    return;
  }

  pushPtr();

  Slot const valSlot = getTemp(((val >> 8) & 0xff) ? literal::i16(val) : literal::i8(val));
  slotLessSlot(lhs, valSlot);

  popPtr();
}

void Assembler::slotLessSlot(Slot const &lhs, Slot const &rhs) {
  assert(not types::isSignedInteger(lhs.type) && not types::isSignedInteger(rhs.type) && "not implemented yet");
  
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

void Assembler::slotLessEqualConst(Slot const &lhs, int val) {
  assert(not types::isSignedInteger(lhs.type) && "not implemented yet");

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
  
  Slot const valSlot = getTemp(((val >> 8) & 0xff) ? literal::i16(val) : literal::i8(val));
  slotLessEqualSlot(lhs, valSlot);

  popPtr();
  
}

void Assembler::slotLessEqualSlot(Slot const &lhs, Slot const &rhs) {
  assert(not types::isSignedInteger(lhs.type) && not types::isSignedInteger(rhs.type) && "not implemented yet");

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


void Assembler::slotGreaterConst(Slot const &lhs, int val) {
  assert(not types::isSignedInteger(lhs.type) && "not implemented yet");

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

  Slot const valSlot = getTemp(((val >> 8) & 0xff) ? literal::i16(val) : literal::i8(val));
  slotGreaterSlot(lhs, valSlot);

  popPtr();
}

void Assembler::slotGreaterSlot(Slot const &lhs, Slot const &rhs) {
  assert(not types::isSignedInteger(lhs.type) && not types::isSignedInteger(rhs.type) && "not implemented yet");
  
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

void Assembler::slotGreaterEqualConst(Slot const &lhs, int val) {
  assert(not types::isSignedInteger(lhs.type) && "not implemented yet");

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

  Slot const valSlot = getTemp(((val >> 8) & 0xff) ? literal::i16(val) : literal::i8(val));
  slotGreaterEqualSlot(lhs, valSlot);

  popPtr();
}

void Assembler::slotGreaterEqualSlot(Slot const &lhs, Slot const &rhs) {
  assert(not types::isSignedInteger(lhs.type) && not types::isSignedInteger(rhs.type) && "not implemented yet");
  
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

void Assembler::eqDestructive(Cell other, Temps<1> tmp) {
  auto [cur, oth] = getFieldIndices(_dp.current(), other);
  emit<primitive::Equal>(cur, oth);
}

void Assembler::eqConstructive(Cell result, Cell other, Temps<1> tmp) {
  pushPtr();
  copyField(result, tmp);
  moveTo(result);
  eqDestructive(other, tmp);
  popPtr();
}

void Assembler::eq16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<1> tmp) {
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

void Assembler::eq16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<4> tmp) { 

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


void Assembler::lessDestructive(Cell other, Temps<2> tmp) { 
  auto [cur, oth, tmp0, tmp1] = getFieldIndices(_dp.current(), other, tmp.get<0>(), tmp.get<1>());
  emit<primitive::Less>(cur, oth, tmp0, tmp1);
}

void Assembler::lessConstructive(Cell result, Cell other, Temps<3> tmp) {
  Cell const &otherCopy = tmp.get<0>();
  pushPtr();
  copyField(result, tmp.select<1>());
  moveTo(other);
  copyField(otherCopy, tmp.select<1>());
  moveTo(result);
  lessDestructive(otherCopy, tmp.select<1, 2>());
  popPtr();
}

void Assembler::less16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<4> tmp) {
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

void Assembler::less16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<8> tmp) {

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


void Assembler::lessOrEqualDestructive(Cell other, Temps<2> tmp) {
  auto [cur, oth, tmp0, tmp1] = getFieldIndices(_dp.current(), other, tmp.get<0>(), tmp.get<1>());
  emit<primitive::LessOrEqual>(cur, oth, tmp0, tmp1);
}

void Assembler::lessOrEqualConstructive(Cell result, Cell other, Temps<3> tmp) {
  Cell const &otherCopy = tmp.get<0>();
  pushPtr();
  copyField(result, tmp.select<1>());
  moveTo(other);
  copyField(otherCopy, tmp.select<1>());
  moveTo(result);
  lessOrEqualDestructive(otherCopy, tmp.select<1, 2>());
  popPtr();
}

void Assembler::lessOrEqual16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<4> tmp) {
  greater16Destructive(high, otherLow, otherHigh, tmp);
  notDestructive(tmp.select<0>());
}

void Assembler::lessOrEqual16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<8> tmp) {

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

void Assembler::greaterDestructive(Cell other, Temps<2> tmp) {
  auto [cur, oth, tmp0, tmp1] = getFieldIndices(_dp.current(), other, tmp.get<0>(), tmp.get<1>());
  emit<primitive::Greater>(cur, oth, tmp0, tmp1);
}

void Assembler::greaterConstructive(Cell result, Cell other, Temps<3> tmp) {
  Cell const &otherCopy = tmp.get<0>();
  pushPtr();
  copyField(result, tmp.select<1>());
  moveTo(other);
  copyField(otherCopy, tmp.select<1>());
  moveTo(result);
  greaterDestructive(otherCopy, tmp.select<1, 2>());
  popPtr();
}


void Assembler::greater16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<4> tmp) {
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

void Assembler::greater16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<8> tmp) {

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



void Assembler::greaterOrEqualDestructive(Cell other, Temps<2> tmp) {
  auto [cur, oth, tmp0, tmp1] = getFieldIndices(_dp.current(), other, tmp.get<0>(), tmp.get<1>());
  emit<primitive::GreaterOrEqual>(cur, oth, tmp0, tmp1);
}

void Assembler::greaterOrEqualConstructive(Cell result, Cell other, Temps<3> tmp) {
  Cell const &otherCopy = tmp.get<0>();
  pushPtr();
  copyField(result, tmp.select<1>());
  moveTo(other);
  copyField(otherCopy, tmp.select<1>());
  moveTo(result);
  greaterOrEqualDestructive(otherCopy, tmp.select<1, 2>());
  popPtr();
}

void Assembler::greaterOrEqual16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<4> tmp) {
  less16Destructive(high, otherLow, otherHigh, tmp);
  notDestructive(tmp.select<0>());
}

void Assembler::greaterOrEqual16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<8> tmp) {

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
