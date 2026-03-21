#include "compiler.ih"

Slot Compiler::getTemp(types::TypeHandle type) {
  assert(_currentBlock != nullptr);

  assert(false);
  std::unreachable();
}

Slot Compiler::getTemp(values::Value const &value) {
  assert(false);
  std::unreachable();
  
}
