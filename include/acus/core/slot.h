#pragma once

#include <string>

#include "acus/types/types_fwd.h"

namespace acus {

struct Slot {

  enum Kind {
    Local,
    Global,
    GlobalReference,
    ArrayElement,
    StructField,
    Dummy,
    Available,
    Temp,
    Invalid
  };

  std::string name;  
  types::TypeHandle type;
  Kind kind;
  int offset;
  void const *scope;
	 
  int size() const;
  operator int() const { return offset; }

  Slot sub(types::TypeHandle subType, int subOffset) const;
  Slot unsignedView() const;

  bool valid() const;
  static Slot invalid();
};

} // namespace acus
