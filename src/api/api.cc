#include "acus/api/context.h"
#include "acus/builder/builder.h"

namespace Impl {
  std::string expected;
}


std::string acus::api::expected() {
  return Impl::expected;
}

void acus::api::clearExpected() {
  Impl::expected = "";
}

void acus::api::expectNext(std::string const &name) {
  Impl::expected = name;
}

acus::api::ExpResult acus::api::isExpected(std::string const &name, bool strict) {
  std::string const &expected = Impl::expected;

  if (expected == name || (!strict && expected.empty())) {
    return { true, "" };
  }

  if (expected.empty()) {
    return {
      .expected = false,
      .msg = "unexpected '" + name + "'."
    };
  }

  return {
    .expected = false,
    .msg = "expected '" + expected + "', but got '" + name + "'."
  };
}

acus::api::Context::Context(Builder const &b, std::string const &name, std::source_location loc):
  _builder(&b),
  _name(name),
  _loc(std::move(loc))
{}

bool acus::api::Context::programStarted() const {
  assert(_builder);
  return _builder->programStarted();
}

std::string acus::api::Context::currentFunction() const {
  assert(_builder);
  return _builder->currentFunction();
}

std::string acus::api::Context::currentBlock() const {
  assert(_builder);
  return _builder->currentBlock();
}

int acus::api::Context::currentScopeDepth() const {
  assert(_builder);
  return _builder->currentScopeDepth();
}

bool acus::api::Context::declaredAsGlobal(std::string const &name) const {
  assert(_builder);
  return _builder->declaredAsGlobal(name);
}

std::string acus::api::Context::apiName() const {
  assert(_builder);
  return _name;
}

bool acus::api::Context::globalDeclarationsAllowed() const {
  assert(_builder);
  return _builder->globalDeclarationsAllowed();
}

bool acus::api::Context::inScope(std::string const &name) const {
  assert(_builder);
  return _builder->inScope(name);
}

bool acus::api::Context::inCurrentScope(std::string const &name) const {
  assert(_builder);
  return _builder->inCurrentScope(name);
}
