#include "compiler.ih"

Expression Compiler::structFieldImpl(Expression const &obj, int fieldIndex, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_IS_STRUCT(obj);
  API_REQUIRE_FIELD_INDEX_IN_BOUNDS(obj, fieldIndex);
  
  auto structType = types::cast<types::StructType>(obj.type());  
  return structFieldImpl(obj, structType->fieldName(fieldIndex), API_FWD);
}

Expression Compiler::structFieldImpl(Expression const &obj, std::string const &fieldName, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_IS_STRUCT(obj);
  API_REQUIRE_IS_FIELD(obj, fieldName);

  if (obj.isLiteral()) {
    auto structVal = values::cast<types::StructType>(obj.literal());
    return Expression{structVal->field(fieldName)};
  }
  
  return Expression{proxy::structField(obj.slot(), fieldName)};
}

Expression Compiler::arrayElementImpl(Expression const &arr, int index, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_IS_ARRAY_OR_STRING(arr);
  API_REQUIRE_INDEX_IN_BOUNDS(arr, index);

  if (arr.isLiteral()) {
    auto arrVal = values::cast<types::ArrayLike>(arr.literal());
    return Expression{arrVal->element(index)};
  }
  
  return Expression{proxy::arrayElement(arr.slot(), index)};  
}  

Expression Compiler::arrayElementImpl(Expression const &arr, Expression const &index, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_IS_ARRAY_OR_STRING(arr);
  API_REQUIRE_IS_INTEGER(index);

  if (index.isLiteral()) {
    int const i = values::cast<types::IntegerType>(index.literal())->value();
    return arrayElementImpl(arr, i, API_FWD);
  }

  if (arr.isLiteral()) {
    Slot const tmp = getTemp(arr.type());
    assignSlot(tmp, arr.literal());
    return Expression{proxy::arrayElement(tmp, index.slot())};
  }
  
  return Expression{proxy::arrayElement(arr.slot(), index.slot())};
}

Expression Compiler::dereferencePointerImpl(Expression const &ptr, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_IS_POINTER(ptr);
  assert(not ptr.isLiteral());
  
  return Expression{proxy::dereferencedPointer(ptr.slot())};
}

