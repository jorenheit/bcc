#include "compiler.ih"

std::string Compiler::currentFunction() const {
  return _currentFunction ? (*_currentFunction).name : "";
}

std::string Compiler::currentBlock() const {
  return _currentBlock ? (*_currentBlock).name : "";
}

bool Compiler::programStarted() const {
  return _state.begun;
}

bool Compiler::declaredAsGlobal(std::string const &name) const {
  return _program.isGlobal(name);
}

bool Compiler::globalDeclarationsAllowed() const {
  return _state.allowGlobalDefinitions; // TODO: rename declarations
}

bool Compiler::inScope(std::string const &name) const {
  return local(name).valid();
}

bool Compiler::inCurrentScope(std::string const &name) const {
  Slot const slot = local(name);
  return slot.valid() && slot.scope == _currentScope;
}

int Compiler::currentScopeDepth() const {
  int depth = 0;
  Function::Scope *scope = _currentScope;
  while (scope != nullptr) {
    scope = scope->parent;
    ++depth;
  }
  return depth;
}
