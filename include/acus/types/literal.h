#pragma once
#include "acus/types/typesystem.h"
#include "acus/types/literal_impl.h"

#define API_HEADER
#include "acus/api/api.h"

// Public literal API (implemented in literal.cc)
namespace acus::literal {  
  class StructLiteralBuilder;
  class ArrayLiteralBuilder;

  using Literal = impl::Literal; 
  
  Literal i8(int val, API_FUNC);
  Literal i16(int val, API_FUNC);
  Literal string(std::string const &str, API_FUNC);
  Literal function_pointer(types::FunctionType const *fType, std::string const &fName, API_FUNC);

  StructLiteralBuilder struct_t(types::TypeHandle structType, API_FUNC);
  ArrayLiteralBuilder array(types::TypeHandle arrayType, API_FUNC);

  template <typename T> requires std::derived_from<T, types::Type>
  auto cast(Literal const &v);
} // namespace acus::literal

#include "literal_cast.tpp"

// Builder classes for arrays and structs (implemented in literal_builders.cc)
namespace acus::literal {
  
  class StructLiteralBuilder {
  public:
    StructLiteralBuilder &init(std::string const &name, Literal val);
    Literal done();
    
    StructLiteralBuilder(types::TypeHandle structType, api::impl::Context const &ctx);
    ~StructLiteralBuilder() noexcept(false);

  private:
    types::TypeHandle _structType;
    api::impl::Context API_CTX_NAME;
    bool _finalized = false;
    std::unordered_map<std::string, Literal> _fields;
    
    StructLiteralBuilder(StructLiteralBuilder const&) = delete;
    StructLiteralBuilder(StructLiteralBuilder&&) = delete;
    StructLiteralBuilder& operator=(StructLiteralBuilder const&) = delete;
    StructLiteralBuilder& operator=(StructLiteralBuilder&&) = delete;
  };

  class ArrayLiteralBuilder {
  public:
    ArrayLiteralBuilder &push(Literal value);
    Literal done();
    ~ArrayLiteralBuilder() noexcept(false);
    ArrayLiteralBuilder(types::TypeHandle arrayType, api::impl::Context const &ctx);

  private:
    types::ArrayType const *_arrayType;
    types::TypeHandle _elementType;
    size_t _length;
    api::impl::Context API_CTX_NAME;
    bool _finalized = false;
    std::vector<Literal> _elements;
  
    ArrayLiteralBuilder(ArrayLiteralBuilder const&) = delete;
    ArrayLiteralBuilder(ArrayLiteralBuilder&&) = delete;
    ArrayLiteralBuilder& operator=(ArrayLiteralBuilder const&) = delete;
    ArrayLiteralBuilder& operator=(ArrayLiteralBuilder&&) = delete;
  };

} // namespace acus::literal

