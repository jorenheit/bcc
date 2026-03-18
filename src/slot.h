#pragma once

#include <memory>
#include "types.h"

struct Slot {

  enum StorageType {
    Local,
    Global,
    GlobalReference,
    ArrayElement
    //    TempUsed,
    //    TempFree
  };

  std::string name;  
  types::TypePtr type;
  StorageType storageType;
  int offset;
	 
  int size() const { return type->size(); }
  operator int() const { return offset; }
};
