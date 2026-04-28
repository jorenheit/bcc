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

