// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

  inline bool operator==(Slot const &s1, Slot const &s2) {
    return s1.offset == s2.offset && s1.size() == s2.size();
  }

  
} // namespace acus
