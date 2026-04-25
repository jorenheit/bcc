#include "bfc/api/context.h"
#include "bfc/compiler/compiler.h"

namespace Impl {
  std::string expected;
}


std::string api::expected() {
  return Impl::expected;
}

void api::clearExpected() {
  Impl::expected = "";
}

void api::expectNext(std::string const &name) {
  Impl::expected = name;
}

bool api::isExpected(std::string const &name, bool strict) {
  if (not strict && Impl::expected.empty()) return true;
  return Impl::expected == name;
}

api::Context::Context(Compiler const &c, std::string const &name, std::source_location loc):
  _compiler(&c),
  _name(name),
  _loc(std::move(loc))
{}

bool api::Context::programStarted() const {
  assert(_compiler);
  return _compiler->programStarted();
}

std::string api::Context::currentFunction() const {
  assert(_compiler);
  return _compiler->currentFunction();
}

std::string api::Context::currentBlock() const {
  assert(_compiler);
  return _compiler->currentBlock();
}

int api::Context::currentScopeDepth() const {
  assert(_compiler);
  return _compiler->currentScopeDepth();
}

bool api::Context::declaredAsGlobal(std::string const &name) const {
  assert(_compiler);
  return _compiler->declaredAsGlobal(name);
}

std::string api::Context::apiName() const {
  assert(_compiler);
  return _name;
}

bool api::Context::globalDeclarationsAllowed() const {
  assert(_compiler);
  return _compiler->globalDeclarationsAllowed();
}

bool api::Context::inScope(std::string const &name) const {
  assert(_compiler);
  return _compiler->inScope(name);
}

bool api::Context::inCurrentScope(std::string const &name) const {
  assert(_compiler);
  return _compiler->inCurrentScope(name);
}
