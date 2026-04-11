#include "compiler.ih"

void Compiler::loopOpen(std::string const &tag) {
  emit<primitive::LoopOpen>(tag);
}

void Compiler::loopClose(std::string const &tag) {
  emit<primitive::LoopClose>(tag);
}

void Compiler::switchField(MacroCell::Field field) {
  assert(_currentSeq != nullptr);
  emit<primitive::MovePointerRelative>(field - _dp.current().field);
  _dp.set(field);
}

void Compiler::moveTo(int offset, MacroCell::Field field) {
  assert(_currentSeq != nullptr);

  switchField(field);
  moveRel(offset - _dp.current().offset);
}

void Compiler::moveTo(Cell dest) {
  moveTo(dest.offset, dest.field);
}

void Compiler::moveRel(int diff) {
  emit<primitive::MovePointerRelative>(diff * MacroCell::FieldCount);
  _dp.moveRelative(diff);
}

void Compiler::moveToOrigin() {
  moveTo(0);
}


void Compiler::zeroCell() { 
  emit<primitive::ZeroCell>();
}

void Compiler::setToValue(int value) { 
  zeroCell();
  addConst(value & 0xff);
}

void Compiler::setToValue16(int value, Cell high) { 
  pushPtr();
  setToValue(value & 0xff);
  moveTo(high);
  setToValue((value >> 8) & 0xff);
  popPtr();
}

void Compiler::addConst(int delta) {
  emit<primitive::ChangeBy>(delta);
}

void Compiler::addConstAndCarry(int delta, Cell carry, Temps<3> tmp) {
  pushPtr();

  if (delta == 0) {
    moveTo(carry);
    zeroCell();
    popPtr();
    return;
  }

  Cell const probe = tmp.get<0>();    
  copyField(carry, tmp.select<1>());
  addConst(delta);
  copyField(probe, tmp.select<1>());

  moveTo(probe);
  if (delta > 0) {
    lessDestructive(carry, tmp.select<1, 2>());
  } else {
    greaterDestructive(carry, tmp.select<1, 2>());
  }

  popPtr();
}  

void Compiler::subConst(int delta) {
  addConst(-delta);
}

void Compiler::mulConst(int factor, Temps<3> tmp) {
  // TODO: optimize for powers of 2
  // TODO: big factors should have runtime implementation
  if (factor == 1) return;

  pushPtr();
  Cell const current = _dp.current();
  Cell const copy1 = tmp.get<0>();
  Cell const copy2 = tmp.get<1>();
  
  copyField(copy1, tmp.select<2>());
  copyField(copy2, tmp.select<2>());
  
  for (int i = 0; i != factor - 1; ++i) {
    addDestructive(copy1);
    moveTo(copy2);
    copyField(copy1, tmp.select<2>());
    moveTo(current);
  }
  popPtr();
}

//void Compiler::mul16Const(int factor, Cell high, Temps<?> tmp);


void Compiler::subConstAndCarry(int delta, Cell carry, Temps<3> tmp) {
  addConstAndCarry(-delta, carry, tmp);
}

void Compiler::inc() {
  addConst(1);
}

void Compiler::dec() {
  subConst(1);
}

void Compiler::inc16(Cell high, Temps<2> tmp) {
  pushPtr();
  Cell const carry = tmp.get<0>();
  addConst(1);
  notConstructive(carry, tmp.select<1>());
  moveTo(high);
  addDestructive(carry);
  popPtr();
}

void Compiler::dec16(Cell high, Temps<2> tmp) {
  pushPtr();
  Cell const borrow = tmp.get<0>();
  copyField(borrow, tmp.select<1>());
  subConst(1);
  moveTo(borrow);
  notDestructive(tmp.select<1>());
  moveTo(high);
  subDestructive(borrow);
  popPtr();
}


void Compiler::add16Const(int delta, Cell high, Temps<4> tmp) {
  if (delta == 0) return;
  
  int const lowDelta  = delta & 0xff;
  int const highDelta = (delta >> 8) & 0xff;
  Cell const carry = tmp.get<0>();

  pushPtr();
  if (lowDelta != 0)  addConstAndCarry(lowDelta, carry, tmp.select<1, 2, 3>());
  moveTo(high);
  if (highDelta != 0) addConst(highDelta);
  addDestructive(carry);
  popPtr();
}


void Compiler::sub16Const(int delta, Cell high, Temps<4> tmp) {
  add16Const(-delta, high, tmp);
}

void Compiler::addDestructive(Cell other) {
  auto [cur, oth] = getFieldIndices(_dp.current(), other);
  emit<primitive::Add>(cur, oth);
}

void Compiler::addConstructive(Cell result, Cell other, Temps<2> tmp) {
  pushPtr();
  copyField(result, tmp.get<0>());
  moveTo(other);
  copyField(tmp.get<0>(), tmp.get<1>());
  moveTo(result);
  addDestructive(tmp.get<0>());
  popPtr();
}

void Compiler::subDestructive(Cell other) {
  auto [cur, oth] = getFieldIndices(_dp.current(), other);
  emit<primitive::Subtract>(cur, oth);
}

void Compiler::subConstructive(Cell result, Cell other, Temps<2> tmp) {
  pushPtr();
  copyField(result, tmp.get<0>());
  moveTo(other);
  copyField(tmp.get<0>(), tmp.get<1>());
  moveTo(result);
  subDestructive(tmp.get<0>());
  popPtr();
}

void Compiler::addAndCarryDestructive(Cell carry, Cell other, Temps<2> tmp) {
  pushPtr();
  copyField(carry, tmp.get<0>());
  addDestructive(other);
  lessDestructive(carry, tmp);
  popPtr();
}

void Compiler::addAndCarryConstructive(Cell result, Cell carry, Cell other, Temps<3> tmp) {
  pushPtr();
  copyField(result, tmp.get<0>());
  moveTo(other);
  copyField(tmp.get<0>(), tmp.get<1>());
  moveTo(result);
  addAndCarryDestructive(carry, tmp.get<0>(), tmp.select<1, 2>()); 
  popPtr();
}

void Compiler::subAndCarryDestructive(Cell carry, Cell other, Temps<2> tmp) {
  pushPtr();
  copyField(carry, tmp.get<0>());
  subDestructive(other);
  greaterDestructive(carry, tmp);
  popPtr();
}

void Compiler::subAndCarryConstructive(Cell result, Cell carry, Cell other, Temps<3> tmp) {
  pushPtr();
  copyField(result, tmp.get<0>());
  moveTo(other);
  copyField(tmp.get<0>(), tmp.get<1>());
  moveTo(result);
  subAndCarryDestructive(carry, tmp.get<0>(), tmp.select<1, 2>()); 
  popPtr();
}


void Compiler::add16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<3> tmp) {

  pushPtr();

  Cell const &low   = _dp.current();
  Cell const &carry = tmp.get<0>();

  // add low bytes and get the carry
  moveTo(low);
  addAndCarryDestructive(carry, otherLow, tmp.select<1, 2>());

  // if carry -> increment high byte
  moveTo(carry);
  loopOpen(); {
    zeroCell(); // reset carry
    moveTo(high);
    addConst(1);
    moveTo(carry);
  } loopClose();
  
  // add high bytes, ignore carry
  moveTo(high);  
  addDestructive(otherHigh);

  popPtr();
}


void Compiler::add16Constructive(Cell high, Cell resultLow, Cell resultHigh, Cell otherLow, Cell otherHigh, Temps<5> tmp) {

  Cell const & low      = _dp.current();
  Cell const & otherLowCopy  = tmp.get<0>();
  Cell const & otherHighCopy = tmp.get<1>();
  
  pushPtr();
  moveTo(low);  copyField(resultLow, tmp.select<2>());
  moveTo(high); copyField(resultHigh, tmp.select<2>());
  moveTo(otherLow);  copyField(otherLowCopy, tmp.select<2>());
  moveTo(otherHigh); copyField(otherHighCopy, tmp.select<2>());

  moveTo(resultLow);
  add16Destructive(resultHigh, otherLowCopy, otherHighCopy, tmp.select<2, 3, 4>());
  popPtr();
}


void Compiler::sub16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<3> tmp) {

  Cell const &low = _dp.current();
  Cell const &carry = tmp.get<0>();

  pushPtr();

  // subtract low bytes and get the carry
  moveTo(low);
  subAndCarryDestructive(carry, otherLow, tmp.select<1, 2>());

  // if carry -> increment high byte
  moveTo(carry);
  loopOpen(); {
    zeroCell(); // reset carry
    moveTo(high);
    subConst(1);
    moveTo(carry);
  } loopClose();
  
  // subtract high bytes, ignore carry
  moveTo(high);  
  subDestructive(otherHigh);

  popPtr();
}

void Compiler::sub16Constructive(Cell high, Cell resultLow, Cell resultHigh, Cell otherLow, Cell otherHigh, Temps<5> tmp) {

  Cell const &low = _dp.current();
  Cell const &otherLowCopy  = tmp.get<0>();
  Cell const &otherHighCopy = tmp.get<1>();
  
  pushPtr();
  moveTo(low);  copyField(resultLow, tmp.select<2>());
  moveTo(high); copyField(resultHigh, tmp.select<2>());
  moveTo(otherLow);  copyField(otherLowCopy, tmp.select<2>());
  moveTo(otherHigh); copyField(otherHighCopy, tmp.select<2>());

  moveTo(resultLow);
  sub16Destructive(resultHigh, otherLowCopy, otherHighCopy, tmp.select<2, 3, 4>());
  popPtr();
}


void Compiler::moveField(Cell dest) {
  auto [src, dst] = getFieldIndices(_dp.current(), dest);
  if (src == dst) return;
  emit<primitive::MoveData>(src, dst);
}

void Compiler::copyField(Cell dest, Temps<1> tmp) {
  auto [src, dst, tmp0] = getFieldIndices(_dp.current(), dest, tmp.get<0>());
  emit<primitive::CopyData>(src, dst, tmp0);
}


void Compiler::notDestructive(Temps<1> tmp) {
  auto [cur, tmp0] = getFieldIndices(_dp.current(), tmp.get<0>());
  emit<primitive::Not>(cur, tmp0);
}

void Compiler::notConstructive(Cell result, Temps<1> tmp) {
  pushPtr();
  copyField(result, tmp);
  moveTo(result);
  notDestructive(tmp);
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

void Compiler::compareToConstDestructive(int value, Temps<1> tmp) {
  auto [cur, tmp0] = getFieldIndices(_dp.current(), tmp.get<0>());
  emit<primitive::Cmp>(value, cur, tmp0);
}

void Compiler::compareToConstConstructive(int value, Cell result, Temps<1> tmp) {
  pushPtr();
  copyField(result, tmp);
  moveTo(result);
  compareToConstDestructive(value, tmp);
  popPtr();
}
    
void Compiler::compare16ToConstDestructive(int value, Cell high, Temps<1> tmp) {
  pushPtr();
  compareToConstDestructive(value & 0xff, tmp);
  moveTo(high);
  compareToConstDestructive((value >> 8) & 0xff, tmp);
  popPtr();

  andDestructive(high, tmp);
}

void Compiler::compare16ToConstConstructive(int value, Cell high, Cell result, Temps<2> tmp) {
  pushPtr();
  copyField(result, tmp.get<1>());
  moveTo(high);
  copyField(tmp.get<0>(), tmp.get<1>());
  moveTo(result);
  compare16ToConstDestructive(value, tmp.get<0>(), tmp.select<1>());
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


void Compiler::equalDestructive(Cell other, Temps<2> tmp) {
  auto [cur, oth, tmp0, tmp1] = getFieldIndices(_dp.current(), other, tmp.get<0>(), tmp.get<1>());
  emit<primitive::Equal>(cur, oth, tmp0, tmp1);
}

void Compiler::equalConstructive(Cell result, Cell other, Temps<3> tmp) {
  Cell const &otherCopy = tmp.get<0>();
  pushPtr();
  copyField(result, tmp.select<1>());
  moveTo(other);
  copyField(otherCopy, tmp.select<1>());
  moveTo(result);
  equalDestructive(otherCopy, tmp.select<1, 2>());
  popPtr();
}

void Compiler::goToDynamicOffset(Cell offsetLow, Cell offsetHigh) {
  // WARNING: this leaves pointer in unknown position.
  // Make sure to leave a marker in order to be able to seek back
  
  // Copy offset (16-bit) into payload cells of the current cell  
  int const base = _dp.current().offset;
  moveTo(offsetLow);
  copyField(Cell{base, MacroCell::Payload0}, Temps<1>::pack(base, MacroCell::Scratch0));
  moveTo(offsetHigh);
  copyField(Cell{base, MacroCell::Payload1}, Temps<1>::pack(base, MacroCell::Scratch0));
  
  // Starting at the current offset, move right while decrementing offset until
  // both bytes have become zero. Then move the value back to the seek-marker
  moveTo(base, MacroCell::Payload0);
  orConstructive(Cell{base, MacroCell::Flag},
		 Cell{base, MacroCell::Payload1},
		 Temps<2>::pack(base, MacroCell::Scratch0,
				base, MacroCell::Scratch1));
  moveTo(base, MacroCell::Flag);
  loopOpen(); {
    zeroCell();

    // Decrement the offset
    switchField(MacroCell::Payload0);
    dec16(Cell{base, MacroCell::Payload1},
	  Temps<2>::pack(base, MacroCell::Scratch0,
			 base, MacroCell::Scratch1));
    
    // move payload forward by 1
    moveField(Cell{base + 1, MacroCell::Payload0});
    switchField(MacroCell::Payload1);
    moveField(Cell{base + 1, MacroCell::Payload1});

    // Follow along with pointer (raw -> compile-time offset remains at base)
    switchField(MacroCell::Payload0);    
    emit<primitive::MovePointerRelative>(MacroCell::FieldCount);

    // Flag <- (payload == 0)
    orConstructive(Cell{base, MacroCell::Flag},
		   Cell{base, MacroCell::Payload1},
		   Temps<2>::pack(base, MacroCell::Scratch0,
				  base, MacroCell::Scratch1));
    switchField(MacroCell::Flag);
  } loopClose();
  moveTo(base, MacroCell::Value0);
}

void Compiler::fetchFromDynamicOffset(Cell offsetLow, Cell offsetHigh, Payload const &payload, primitive::Direction seekDir) {
  assert(payload);

  int const base = _dp.current().offset;
  pushPtr();
  goToDynamicOffset(offsetLow, offsetHigh);
  
  // Base is now the cell we arrived at (at offset).
  // Load values into payload

  int baseOffset = 0;
  for (Payload::Unit const &unit: payload.units) {
    for (int i = 0; i != unit.size; ++i) {
      moveTo(base + baseOffset, MacroCell::Value0);
      copyField(Cell{base + baseOffset, MacroCell::Payload0}, Temps<1>::pack(base + baseOffset, MacroCell::Scratch0));
      if (unit.width == Payload::Width::Double) {
	moveTo(base + baseOffset, MacroCell::Value1);
	copyField(Cell{base + baseOffset, MacroCell::Payload1}, Temps<1>::pack(base + baseOffset, MacroCell::Scratch0));
      }
      ++baseOffset;
    }
  }
  
  // Bring payload back to cell that contains the SeekMarker
  moveTo(base);
  seek(MacroCell::SeekMarker, seekDir, payload, true);
  popPtr();

  // Transfer complete: payload now in Payload-fields of the base
}
