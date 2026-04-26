#include "acus/core/slot.h"
#include "acus/types/types.h"

using namespace acus;

int Slot::size() const { return type->size(); }

Slot Slot::sub(types::TypeHandle subType, int subOffset) const {
  return Slot {
    .name = name + "<" + std::to_string(subOffset) + ">",
    .type = subType,
    .kind = Dummy,
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
