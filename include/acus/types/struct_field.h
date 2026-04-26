#pragma once

#include <string>
#include <vector>

#include "acus/types/types_fwd.h"

namespace acus {

// TODO: remove
struct StructField {
  std::string name;
  types::TypeHandle type;
};

using StructFields = std::vector<StructField>;

struct NameTypePair {
  std::string name;
  types::TypeHandle type;
};

} // namespace acus