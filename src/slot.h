#pragma once

#include <memory>
#include <functional>
#include "types.h"

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
	 
  int size() const { return type->size(); }
  operator int() const { return offset; }

  Slot sub(types::TypeHandle subType, int subOffset) const {
    return Slot {
      .name = name + "<" + std::to_string(subOffset) + ">",
      .type = subType,
      .kind = Dummy,
      .offset = offset + subOffset,
      .scope = scope
    };
  }

  bool valid() const { return kind != Invalid; }
  
  static Slot invalid() {
    return Slot {
      .name = "",
      .type = types::null,
      .kind = Invalid,
      .offset = 0,
      .scope = nullptr
    };
  }
};


