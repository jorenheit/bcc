#include "builder.ih"

std::string Builder::currentFunction() const {
  return _currentFunction ? (*_currentFunction).name : "";
}

std::string Builder::currentBlock() const {
  return _currentBlock ? (*_currentBlock).name : "";
}

bool Builder::programStarted() const {
  return _state.begun;
}

bool Builder::declaredAsGlobal(std::string const &name) const {
  return _program.isGlobal(name);
}

bool Builder::globalDeclarationsAllowed() const {
  return _state.allowGlobalDefinitions; // TODO: rename declarations
}

bool Builder::inScope(std::string const &name) const {
  return local(name).valid();
}

bool Builder::inCurrentScope(std::string const &name) const {
  Slot const slot = local(name);
  return slot.valid() && slot.scope == _currentScope;
}

int Builder::currentScopeDepth() const {
  int depth = 0;
  Function::Scope *scope = _currentScope;
  while (scope != nullptr) {
    scope = scope->parent;
    ++depth;
  }
  return depth;
}
