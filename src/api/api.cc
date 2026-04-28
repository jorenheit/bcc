#include "acus/api/context.h"
#include "acus/builder/builder.h"

namespace Impl {
  std::string expected;
}


std::string acus::api::impl::expected() {
  return Impl::expected;
}

void acus::api::impl::clearExpected() {
  Impl::expected = "";
}

void acus::api::impl::expectNext(std::string const &name) {
  Impl::expected = name;
}

acus::api::impl::ExpResult acus::api::impl::isExpected(std::string const &name, bool strict) {
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

acus::api::impl::Context::Context(Builder const &b, std::string const &name, std::source_location loc):
  _builder(&b),
  _name(name),
  _loc(std::move(loc))
{}

acus::api::impl::Context::Context(std::string const &name, std::source_location loc):
  _builder(nullptr),
  _name(name),
  _loc(std::move(loc))
{}

bool acus::api::impl::Context::programStarted() const {
  assert(_builder);
  return _builder->programStarted();
}

std::string acus::api::impl::Context::currentFunction() const {
  assert(_builder);
  return _builder->currentFunction();
}

std::string acus::api::impl::Context::currentBlock() const {
  assert(_builder);
  return _builder->currentBlock();
}

int acus::api::impl::Context::currentScopeDepth() const {
  assert(_builder);
  return _builder->currentScopeDepth();
}

bool acus::api::impl::Context::declaredAsGlobal(std::string const &name) const {
  assert(_builder);
  return _builder->declaredAsGlobal(name);
}

std::string acus::api::impl::Context::apiName() const {
  assert(_builder);
  return _name;
}

bool acus::api::impl::Context::globalDeclarationsAllowed() const {
  assert(_builder);
  return _builder->globalDeclarationsAllowed();
}

bool acus::api::impl::Context::inScope(std::string const &name) const {
  assert(_builder);
  return _builder->inScope(name);
}

bool acus::api::impl::Context::inCurrentScope(std::string const &name) const {
  assert(_builder);
  return _builder->inCurrentScope(name);
}
