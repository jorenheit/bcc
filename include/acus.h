#pragma once

#include "acus/types/typesystem.h"
#include "acus/types/literal.h"
#include "acus/assembler/assembler.h"

namespace acus::api {
  using acus::Assembler;
  using acus::Expression;
  using acus::ts::TypeHandle;
  using acus::literal::Literal;
  
  namespace literal { using namespace acus::literal; }
  namespace ts { using namespace acus::ts; }
}
