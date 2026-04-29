#include "acus/types/literal.h"

namespace acus::literal {
  
  StructLiteralBuilder &StructLiteralBuilder::init(std::string const &name, Literal val) {
      API_REQUIRE(not _fields.contains(name), "field '", name, "' was initialized multiple times.");
      _fields[name] = val;
      return *this;
    }

  Literal StructLiteralBuilder::done() {
    _finalized = true;
    return std::make_shared<impl::structT>(_structType, _fields, API_FWD);
  }

  StructLiteralBuilder::StructLiteralBuilder(types::TypeHandle structType, api::impl::Context const &ctx):
    _structType(structType),
    API_CTX_NAME(ctx)
  {}
    
  StructLiteralBuilder::~StructLiteralBuilder() noexcept(false) {
    if (std::uncaught_exceptions() == 0) 
      API_REQUIRE(_finalized, "struct-literal was not finalized; call done() on the builder to get the type-handle.");
  }

  ArrayLiteralBuilder &ArrayLiteralBuilder::push(Literal value) {
    API_REQUIRE(_elements.size() < _length, "too many elements for this array-type.");
    API_EXPECT_TYPE(value->type(), _elementType);
    _elements.push_back(value);
    return *this;
  }
    
  Literal ArrayLiteralBuilder::done() {
    API_REQUIRE(_elements.size() == _length, "too few elements for this array-type.");
    _finalized = true;
    return std::make_shared<impl::array>(_elementType, _elements, API_FWD);
  }
  
  ArrayLiteralBuilder::~ArrayLiteralBuilder() noexcept(false) {
    if (std::uncaught_exceptions() == 0) 
      API_REQUIRE(_finalized, "array-literal was not finalized; call done() on the builder to get the type-handle.");
  }

  ArrayLiteralBuilder::ArrayLiteralBuilder(types::TypeHandle arrayType, api::impl::Context const &ctx):
    API_CTX_NAME(ctx)
  {
    API_REQUIRE(types::isArray(arrayType), "expected an array, got ", arrayType->str());
    _arrayType = types::cast<types::ArrayType>(arrayType);
    _elementType = _arrayType->elementType();
    _length = _arrayType->length();
  }
}
