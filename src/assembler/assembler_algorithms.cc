#include "assembler.ih"

void Assembler::loopOpen(std::string const &tag) {
  emit<primitive::LoopOpen>(tag);
}

void Assembler::loopClose(std::string const &tag) {
  emit<primitive::LoopClose>(tag);
}

void Assembler::switchField(MacroCell::Field field) {
  assert(_currentSeq != nullptr);
  emit<primitive::MovePointerRelative>(field - _dp.current().field);
  _dp.set(field);
}

void Assembler::moveTo(int offset, MacroCell::Field field) {
  assert(_currentSeq != nullptr);

  switchField(field);
  moveRel(offset - _dp.current().offset);
}

void Assembler::moveTo(Cell dest) {
  moveTo(dest.offset, dest.field);
}

void Assembler::moveRel(int diff) {
  emit<primitive::MovePointerRelative>(diff * MacroCell::FieldCount);
  _dp.moveRelative(diff);
}

void Assembler::moveToOrigin() {
  moveTo(0);
}


void Assembler::zeroCell() { 
  emit<primitive::ZeroCell>();
}

void Assembler::setToValue(int value) { 
  zeroCell();
  addConst(value & 0xff);
}

void Assembler::setToValue16(int value, Cell high) { 
  pushPtr();
  setToValue(value & 0xff);
  moveTo(high);
  setToValue((value >> 8) & 0xff);
  popPtr();
}

void Assembler::inc() {
  addConst(1);
}

void Assembler::dec() {
  subConst(1);
}

void Assembler::inc16(Cell high, Temps<2> tmp) {
  pushPtr();
  Cell const carry = tmp.get<0>();
  addConst(1);
  notConstructive(carry, tmp.select<1>());
  moveTo(high);
  addDestructive(carry);
  popPtr();
}

void Assembler::dec16(Cell high, Temps<2> tmp) {
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

void Assembler::moveField(Cell dest) {
  auto [src, dst] = getFieldIndices(_dp.current(), dest);
  if (src == dst) return;
  emit<primitive::MoveData>(src, dst);
}

void Assembler::copyField(Cell dest, Temps<1> tmp) {
  auto [src, dst, tmp0] = getFieldIndices(_dp.current(), dest, tmp.get<0>());
  emit<primitive::CopyData>(src, dst, tmp0);
}

void Assembler::boolDestructive(Temps<1> tmp) {
  auto [current, tmp0] = getFieldIndices(_dp.current(), tmp.get<0>());
  emit<primitive::Boolean>(current, tmp0);
}

void Assembler::bool16Destructive(Cell high, Temps<1> tmp) {
  orDestructive(high, tmp.select<0>());
}

void Assembler::boolConstructive(Cell result, Temps<1> tmp) {
  pushPtr();
  copyField(result, tmp);
  moveTo(result);
  boolDestructive(tmp);
  popPtr();
}

void Assembler::bool16Constructive(Cell high, Cell result, Temps<2> tmp) {
  Cell const resultHigh = tmp.get<0>();
  
  pushPtr();
  copyField(result, tmp.select<1>());
  moveTo(high);
  copyField(resultHigh, tmp.select<1>());
  moveTo(result);
  bool16Destructive(resultHigh, tmp.select<1>());
  popPtr();  
}


void Assembler::notDestructive(Temps<1> tmp) {
  auto [cur, tmp0] = getFieldIndices(_dp.current(), tmp.get<0>());
  emit<primitive::Not>(cur, tmp0);
}

void Assembler::not16Destructive(Cell high, Temps<1> tmp) {
  orDestructive(high, tmp.select<0>());
  notDestructive(tmp.select<0>());
}

void Assembler::notConstructive(Cell result, Temps<1> tmp) {
  pushPtr();
  copyField(result, tmp);
  moveTo(result);
  notDestructive(tmp);
  popPtr();
}

void Assembler::not16Constructive(Cell high, Cell result, Temps<2> tmp) {
  Cell const resultHigh = tmp.get<0>();
  
  pushPtr();
  copyField(result, tmp.select<1>());
  moveTo(high);
  copyField(resultHigh, tmp.select<1>());
  moveTo(result);
  not16Destructive(resultHigh, tmp.select<1>());
  popPtr();  
}

// TODO: remove compareToConst 
void Assembler::compareToConstDestructive(int value, Temps<1> tmp) {
  auto [cur, tmp0] = getFieldIndices(_dp.current(), tmp.get<0>());
  emit<primitive::Cmp>(value, cur, tmp0);
}

void Assembler::compareToConstConstructive(int value, Cell result, Temps<1> tmp) {
  pushPtr();
  copyField(result, tmp);
  moveTo(result);
  compareToConstDestructive(value, tmp);
  popPtr();
}
    
void Assembler::compare16ToConstDestructive(int value, Cell high, Temps<1> tmp) {
  pushPtr();
  compareToConstDestructive(value & 0xff, tmp);
  moveTo(high);
  compareToConstDestructive((value >> 8) & 0xff, tmp);
  popPtr();

  andDestructive(high, tmp);
}

void Assembler::compare16ToConstConstructive(int value, Cell high, Cell result, Temps<2> tmp) {
  pushPtr();
  copyField(result, tmp.get<1>());
  moveTo(high);
  copyField(tmp.get<0>(), tmp.get<1>());
  moveTo(result);
  compare16ToConstDestructive(value, tmp.get<0>(), tmp.select<1>());
  popPtr();
}

void Assembler::goToDynamicOffset(Cell offsetLow, Cell offsetHigh) {
  // WARNING: this leaves pointer in unknown position.
  // Make sure to leave a marker in order to be able to seek back
  
  // Copy offset (16-bit) into payload cells of the current cell  
  int const base = _dp.current().offset;
  moveTo(offsetLow);
  copyField(Cell{base, MacroCell::Payload0}, Temps<1>::select(base, MacroCell::Scratch0));
  moveTo(offsetHigh);
  copyField(Cell{base, MacroCell::Payload1}, Temps<1>::select(base, MacroCell::Scratch0));
  
  // Starting at the current offset, move right while decrementing offset until
  // both bytes have become zero. Then move the value back to the seek-marker
  moveTo(base, MacroCell::Payload0);
  orConstructive(Cell{base, MacroCell::Flag},
		 Cell{base, MacroCell::Payload1},
		 Temps<2>::select(base, MacroCell::Scratch0,
				base, MacroCell::Scratch1));
  moveTo(base, MacroCell::Flag);
  loopOpen(); {
    zeroCell();

    // Decrement the offset
    switchField(MacroCell::Payload0);
    dec16(Cell{base, MacroCell::Payload1},
	  Temps<2>::select(base, MacroCell::Scratch0,
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
		   Temps<2>::select(base, MacroCell::Scratch0,
				  base, MacroCell::Scratch1));
    switchField(MacroCell::Flag);
  } loopClose();
  moveTo(base, MacroCell::Value0);
}

void Assembler::fetchFromDynamicOffset(Cell offsetLow, Cell offsetHigh, Payload const &payload, primitive::Direction seekDir) {
  assert(payload);

  int const base = _dp.current().offset;
  pushPtr();
  goToDynamicOffset(offsetLow, offsetHigh);
  
  // Base is now the cell we arrived at (at offset).
  // Load values into payload
  for (int i = 0; i != payload.size(); ++i) {
    moveTo(base + i, MacroCell::Value0);
    copyField(Cell{base + i, MacroCell::Payload0}, Temps<1>::select(base + i, MacroCell::Scratch0));
    if (payload.width(i) == Payload::Width::Double) {
      moveTo(base + i, MacroCell::Value1);
      copyField(Cell{base + i, MacroCell::Payload1}, Temps<1>::select(base + i, MacroCell::Scratch0));
    }
  }
  
  // Bring payload back to cell that contains the SeekMarker
  moveTo(base);
  seek(MacroCell::SeekMarker, seekDir, payload, true);
  popPtr();

  // Transfer complete: payload now in Payload-fields of the base
}
