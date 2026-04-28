#pragma once
#include <memory>
#include <vector>
#include <cassert>
#include <string>
#include <utility>
#include <type_traits>
#include <concepts>
#include <sstream>
#include "acus/types/typesystem.h"
#include "acus/types/literal_impl.h"

#define API_HEADER
#include "acus/api/api.h"

namespace acus::literal {  

  using Literal = impl::Literal; 
  
  Literal i8(int val, API_FUNC);
  Literal i16(int val, API_FUNC);
  Literal string(std::string const &str, API_FUNC);
  Literal function_pointer(types::FunctionType const *fType, std::string const &fName, API_FUNC);

  [[nodiscard("call the returned StructLiteral object with arguments, e.g. structT(...)(args...)")]]
  impl::StructLiteral structT(types::TypeHandle structType, API_FUNC);

  [[nodiscard("call the returned arrayLiteral object with arguments, e.g. array(i8)(args...)")]]  
  impl::ArrayLiteral array(types::TypeHandle elementType, API_FUNC);

  template <typename T> requires std::derived_from<T, types::Type>
  auto cast(Literal const &v);

#include "literal_public.tpp"
  
} // namespace acus::literal

