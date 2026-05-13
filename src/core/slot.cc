#include "acus/core/slot.h"
#include "acus/types/typesystem.h"

using namespace acus;

int Slot::size() const { return type->size(); }

Slot Slot::sub(types::TypeHandle subType, int subOffset) const {
  return Slot {
    .name = name + "<" + std::to_string(subOffset) + ">",
    .type = subType,
    .kind = this->kind == Temp ? Temp : Dummy,
    .offset = offset + subOffset,
    .scope = scope
  };
}

bool Slot::valid() const { return kind != Invalid; }
  
Slot Slot::invalid() {
  return Slot {
    .name = "",
    .type = types::null,
    .kind = Invalid,
    .offset = 0,
    .scope = nullptr
  };
}

Slot Slot::unsignedView() const {
  assert(types::isInteger(type));
  Slot view = *this;
  view.type = type->usesValue1() ? ts::i16() : ts::i8();
  return view;
}
