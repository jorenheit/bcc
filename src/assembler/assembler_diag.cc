#include "assembler.ih"

std::string Assembler::currentFunction() const {
  return _currentFunction ? (*_currentFunction).name : "";
}

std::string Assembler::currentBlock() const {
  return _currentBlock ? (*_currentBlock).name : "";
}

bool Assembler::programStarted() const {
  return _state.begun;
}

bool Assembler::declaredAsGlobal(std::string const &name) const {
  return _program.isGlobal(name);
}

bool Assembler::globalDeclarationsAllowed() const {
  return _state.allowGlobalDeclarations;
}

bool Assembler::inScope(std::string const &name) const {
  return local(name).valid();
}

bool Assembler::inCurrentScope(std::string const &name) const {
  Slot const slot = local(name);
  return slot.valid() && slot.scope == _currentScope;
}

int Assembler::currentScopeDepth() const {
  int depth = 0;
  Function::Scope *scope = _currentScope;
  while (scope != nullptr) {
    scope = scope->parent;
    ++depth;
  }
  return depth;
}
