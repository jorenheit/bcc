#include "compiler.ih"

void Compiler::loopOpen(std::string const &tag) {
  emit<primitive::LoopOpen>(tag);
}

void Compiler::loopClose(std::string const &tag) {
  emit<primitive::LoopClose>(tag);
}

void Compiler::moveTo(int frameOffset) {
  assert(_currentSeq != nullptr);  
  assert(_dp.isStatic());
  if (frameOffset == -1) return;
  
  int const diff = frameOffset - _dp.staticOffset();
  if (diff == 0) return;

  emit<primitive::MovePointerRelative>(diff * MacroCell::FieldCount);
  _dp.moveRelative(diff);
}

void Compiler::moveTo(int frameOffset, MacroCell::Field field) {
  moveTo(frameOffset);
  switchField(field);
}

void Compiler::moveToOrigin() {
  moveTo(0);
}

void Compiler::switchField(MacroCell::Field field) {
  assert(_currentSeq != nullptr);
  
  auto current = static_cast<int>(_dp.activeField());
  auto target = static_cast<int>(field);
  if (current == target) return;
      
  emit<primitive::MovePointerRelative>(target - current);
  _dp.setField(field);
}


void Compiler::zeroCell() { 
  emit<primitive::ZeroCell>();
}

void Compiler::addConst(int delta) {
  emit<primitive::ChangeBy>(delta);
}

void Compiler::subConst(int delta) {
  addConst(-delta);
}

void Compiler::setToValue(uint8_t value) { 
  zeroCell();
  addConst(value);
}

void Compiler::moveField(int destOffset, MacroCell::Field destField) {

  auto [source, dest] = getFieldIndices(_dp.staticOffset(), _dp.activeField(),
					destOffset, destField);
  emit<primitive::MoveData>(source, dest);
}


void Compiler::moveField(int destOffset1, MacroCell::Field destField1, int destOffset2, MacroCell::Field destField2) {

  auto [source, dest1, dest2] = getFieldIndices(_dp.staticOffset(), _dp.activeField(),
						destOffset1, destField1,
						destOffset2, destField2);
  emit<primitive::MoveData2>(source, dest1, dest2);
}


void Compiler::copyField(int destOffset, MacroCell::Field destField) {
  int const currentOffset = _dp.staticOffset();
  int const currentField = _dp.activeField();
  int const scratchField = pickScratchField(currentField, destField);

  auto [source, dest, tmp] = getFieldIndices(currentOffset, currentField,
					     destOffset, destField,
					     destOffset, scratchField);
  
  emit<primitive::CopyData>(source, dest, tmp);
}


void Compiler::notValue() {
  int const currentOffset = _dp.staticOffset();
  int const currentField = _dp.activeField();
  int const scratch = (currentField != MacroCell::Scratch0) ? MacroCell::Scratch0 : MacroCell::Scratch1;

  auto [current, tmp] = getFieldIndices(currentOffset, currentField,
					currentOffset, scratch);
  emit<primitive::Not1>(current, tmp);
}

void Compiler::notValue(MacroCell::Field resultField) {
  int const currentOffset = _dp.staticOffset();
  int const currentField = _dp.activeField();

  assert(resultField != MacroCell::Scratch0);
  assert(resultField != MacroCell::Scratch1);
  assert(currentField != MacroCell::Scratch0);
  assert(currentField != MacroCell::Scratch1);

  auto [current, result, tmp1, tmp2] = getFieldIndices(currentOffset, currentField,
						       currentOffset, resultField,
						       currentOffset, MacroCell::Scratch0,
						       currentOffset, MacroCell::Scratch1);
  
  emit<primitive::Not2>(current, result, tmp1, tmp2);
}


void Compiler::cmpConst(int value, MacroCell::Field resultField) {
  int const currentOffset = _dp.staticOffset();
  int const currentField = _dp.activeField();

  assert(resultField != MacroCell::Scratch0);
  assert(resultField != MacroCell::Scratch1);
  assert(currentField != MacroCell::Scratch0);
  assert(currentField != MacroCell::Scratch1);

  auto [current, result, tmp1, tmp2] = getFieldIndices(currentOffset, currentField,
						       currentOffset, resultField,
						       currentOffset, MacroCell::Scratch0,
						       currentOffset, MacroCell::Scratch1);
  
  emit<primitive::Cmp>(value, current, result, tmp1, tmp2);
}
