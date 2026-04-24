#include "compiler.ih"

// TODO: group binops in files: compiler_mul.cc, ...

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

void Compiler::moveField(Cell dest) {
  auto [src, dst] = getFieldIndices(_dp.current(), dest);
  if (src == dst) return;
  emit<primitive::MoveData>(src, dst);
}

void Compiler::copyField(Cell dest, Temps<1> tmp) {
  auto [src, dst, tmp0] = getFieldIndices(_dp.current(), dest, tmp.get<0>());
  emit<primitive::CopyData>(src, dst, tmp0);
}

void Compiler::boolDestructive(Temps<1> tmp) {
  auto [current, tmp0] = getFieldIndices(_dp.current(), tmp.get<0>());
  emit<primitive::Boolean>(current, tmp0);
}

void Compiler::bool16Destructive(Cell high, Temps<1> tmp) {
  orDestructive(high, tmp.select<0>());
}

void Compiler::boolConstructive(Cell result, Temps<1> tmp) {
  pushPtr();
  copyField(result, tmp);
  moveTo(result);
  boolDestructive(tmp);
  popPtr();
}

void Compiler::bool16Constructive(Cell high, Cell result, Temps<2> tmp) {
  Cell const resultHigh = tmp.get<0>();
  
  pushPtr();
  copyField(result, tmp.select<1>());
  moveTo(high);
  copyField(resultHigh, tmp.select<1>());
  moveTo(result);
  bool16Destructive(resultHigh, tmp.select<1>());
  popPtr();  
}


void Compiler::notDestructive(Temps<1> tmp) {
  auto [cur, tmp0] = getFieldIndices(_dp.current(), tmp.get<0>());
  emit<primitive::Not>(cur, tmp0);
}

void Compiler::not16Destructive(Cell high, Temps<1> tmp) {
  orDestructive(high, tmp.select<0>());
  notDestructive(tmp.select<0>());
}

void Compiler::notConstructive(Cell result, Temps<1> tmp) {
  pushPtr();
  copyField(result, tmp);
  moveTo(result);
  notDestructive(tmp);
  popPtr();
}

void Compiler::not16Constructive(Cell high, Cell result, Temps<2> tmp) {
  Cell const resultHigh = tmp.get<0>();
  
  pushPtr();
  copyField(result, tmp.select<1>());
  moveTo(high);
  copyField(resultHigh, tmp.select<1>());
  moveTo(result);
  not16Destructive(resultHigh, tmp.select<1>());
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

void Compiler::xorDestructive(Cell other, Temps<1> tmp) {
  auto [cur, oth, tmp0] = getFieldIndices(_dp.current(), other, tmp.get<0>());
  emit<primitive::Xor>(cur, oth, tmp0);
}

void Compiler::xorConstructive(Cell result, Cell other, Temps<2> tmp) { 
  Cell const &otherCopy = tmp.get<0>();
  pushPtr();
  copyField(result, tmp.select<1>());  
  moveTo(other);
  copyField(otherCopy, tmp.select<1>());
  moveTo(result);
  xorDestructive(otherCopy, tmp.select<1>());
  popPtr();
}

void Compiler::xor16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<1> tmp) {

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
  xorDestructive(otherLow, tmp);
  popPtr();
}


void Compiler::xor16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<4> tmp) { 

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
  xor16Destructive(resultHigh, otherCopyLow, otherCopyHigh, tmp.select<3>());
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

void Compiler::xnorDestructive(Cell other, Temps<1> tmp) {
  xorDestructive(other, tmp);
  notDestructive(tmp);
}

void Compiler::xnorConstructive(Cell result, Cell other, Temps<2> tmp) {
  xorConstructive(result, other, tmp);
  notDestructive(tmp.select<0>());
}

void Compiler::xnor16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<1> tmp) {
  xor16Destructive(high, otherLow, otherHigh, tmp);
  notDestructive(tmp);
}

void Compiler::xnor16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<4> tmp) {
  xor16Constructive(high, result, otherLow, otherHigh, tmp);
  notDestructive(tmp.select<0>());
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

void Compiler::fetchFromDynamicOffset(Cell offsetLow, Cell offsetHigh, Payload const &payload, primitive::Direction seekDir) {
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
