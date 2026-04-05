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
  int const diff = offset - _dp.current().offset;
  emit<primitive::MovePointerRelative>(diff * MacroCell::FieldCount);
  _dp.moveRelative(diff);
}

void Compiler::moveTo(Cell dest) {
  moveTo(dest.offset, dest.field);
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

void Compiler::subConst(int delta) {
  addConst(-delta);
}

void Compiler::add16Const(int delta, Cell high, Temps<5> tmp) {
  Cell const &deltaLow  = tmp.get<0>();
  Cell const &deltaHigh = tmp.get<1>();

  pushPtr();
  moveTo(deltaLow);
  setToValue16(delta, deltaHigh);
  popPtr();

  add16Destructive(high, deltaLow, deltaHigh, tmp.select<2, 3, 4>());
}


void Compiler::sub16Const(int delta, Cell high, Temps<5> tmp) {
  Cell const &deltaLow  = tmp.get<0>();
  Cell const &deltaHigh = tmp.get<1>();

  pushPtr();
  moveTo(deltaLow);
  setToValue16(delta, deltaHigh);
  popPtr();

  sub16Destructive(high, deltaLow, deltaHigh, tmp.select<2, 3, 4>());
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

  Cell const & low = _dp.current();
  Cell const & carry = tmp.get<0>();

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



