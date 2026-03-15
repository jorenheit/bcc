#include "compiler.ih"

Slot &Compiler::allocateTemp(int size) {
  assert(_currentBlock != nullptr);

  Slot &slot = declareLocal("__tmp__", size);
  slot.type = Slot::TempUsed;
  return slot;
}

void Compiler::freeTemp(Slot &slot) {
  assert(_currentBlock != nullptr);
  assert(slot.type == Slot::TempUsed);
  slot.type = Slot::TempFree;
}
