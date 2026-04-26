#pragma once
#include <cassert>
#include <string>
#include <variant>
#include "bfc/core/proxy.h"

class Expression {
  std::variant<SlotProxy, values::Literal> _val;
  types::TypeHandle _type;
    
public:
  explicit Expression(Slot const &s): _val(s), _type(s.type) {}
  explicit Expression(SlotProxy const &s): _val(s), _type(s->type()) {}
  explicit Expression(values::Literal const &v): _val(v), _type(v->type()) {}
  Expression(Expression const &) = default;

  bool hasSlot() const   { return std::holds_alternative<SlotProxy>(_val); }
  bool isLiteral() const { return std::holds_alternative<values::Literal>(_val); }
    
  SlotProxy const &slot() const {
    assert(hasSlot());
    return std::get<SlotProxy>(_val);
  }

  values::Literal literal() const {
    assert(isLiteral());
    return std::get<values::Literal>(_val);
  }
    
  types::TypeHandle type() const { return _type; }

  std::string str() const {
    if (hasSlot()) return slot()->name();
    return literal()->str();
  }
};
