#pragma once

#include <memory>
#include "types.h"

struct Slot {

  enum StorageType {
    Local,
    Global,
    GlobalReference,
    TempUsed,
    TempFree
  };

  std::string name;  
  std::shared_ptr<types::Type> type;
  StorageType storageType;
  int offset;
	 
  int size() const { return type->size(); }
  operator int() const { return offset; }
};
