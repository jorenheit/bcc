#pragma once
#include <variant>
#include "proxy.h"

namespace values {

  class RValue {
    // Need to extend the variant to be able to hold an AccessProxy
    std::variant<SlotProxy, values::Anonymous> val;

  public:
    explicit RValue(Slot const &s): val(s) {}
    explicit RValue(SlotProxy const &s): val(s) {}
    explicit RValue(values::Anonymous const &v): val(v) { assert(not v->isRef()); }
    RValue(RValue const &) = default;

    bool hasSlot() const { return std::holds_alternative<SlotProxy>(val); }
    SlotProxy const &slot() const { assert(hasSlot()); return std::get<SlotProxy>(val); }
    values::Anonymous value() const { assert(not hasSlot()); return std::get<values::Anonymous>(val); }
    types::TypeHandle type() const { return hasSlot() ? slot()->type() : value()->type(); }
    std::string str() const { return hasSlot() ? slot()->name() : value()->str(); }
  };

  class LValue {
    SlotProxy _slot;
  public:
    explicit LValue(Slot const &s): _slot(s) {}
    explicit LValue(SlotProxy const &s): _slot(s) {}
    LValue(LValue const &) = default;
    
    SlotProxy const &slot() const { return _slot; }
    types::TypeHandle type() const { return _slot->type(); }
    std::string str() const { return _slot->name(); }
    operator RValue() const { return RValue{_slot}; }
  };

}
