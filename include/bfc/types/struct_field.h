#pragma once

#include <string>
#include <vector>

#include "bfc/types/types_fwd.h"

struct StructField {
  std::string name;
  types::TypeHandle type;
};

using StructFields = std::vector<StructField>;
