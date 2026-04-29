#pragma once
#include <cassert>
#include <string>
#include <variant>
#include "acus/core/proxy.h"
#include "acus/types/literal.h"

namespace acus {

class Expression {
  std::variant<SlotProxy, literal::Literal> _val;
  types::TypeHandle _type;
    
public:
  explicit Expression(Slot const &s): _val(s), _type(s.type) {}
  explicit Expression(SlotProxy const &s): _val(s), _type(s->type()) {}
  explicit Expression(literal::Literal const &v): _val(v), _type(v->type()) {}
  Expression(Expression const &) = default;

  bool hasSlot() const   { return std::holds_alternative<SlotProxy>(_val); }
  bool isLiteral() const { return std::holds_alternative<literal::Literal>(_val); }
    
  SlotProxy const &slot() const {
    assert(hasSlot());
    return std::get<SlotProxy>(_val);
  }

  literal::Literal literal() const {
    assert(isLiteral());
    return std::get<literal::Literal>(_val);
  }
    
  types::TypeHandle type() const { return _type; }

  std::string str() const {
    if (hasSlot()) return slot()->name();
    return literal()->str();
  }
};

} // namespace acus
