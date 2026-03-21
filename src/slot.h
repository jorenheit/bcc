#pragma once

#include <memory>
#include "types.h"

struct Slot {

  enum Kind {
    Local,
    Global,
    GlobalReference,
    ArrayElement,
    Dummy,
    Available
  };

  std::string name;  
  types::TypeHandle type;
  Kind kind;
  int offset;
  void const *scope;
	 
  int size() const { return type->size(); }
  operator int() const { return offset; }
};
