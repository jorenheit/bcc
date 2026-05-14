// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace acus::types {
  struct Type;
  struct VoidType;
  struct RawType;
  struct IntegerType;
  struct ArrayLike;
  struct ArrayType;
  struct StringType;
  struct StructType;
  struct PointerType;
  struct FunctionType;
  using TypeHandle = Type const *;
}

