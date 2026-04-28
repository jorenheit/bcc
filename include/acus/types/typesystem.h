#pragma once
#include "acus/types/types.h"
#include "acus/types/typesystem_fwd.h"

namespace acus::ts {

  void init();

  types::VoidType const *voidT(API_FUNC);
  types::IntegerType const *i8(API_FUNC);
  types::IntegerType const *i16(API_FUNC);
  types::ArrayType const *array(types::TypeHandle elem, int length, API_FUNC);
  types::StringType const *string(int maxLen, API_FUNC);    
  types::RawType const *raw(int n, API_FUNC);
  types::StructType const *structT(std::string const &name, API_FUNC);
  types::PointerType const *pointer(types::TypeHandle pointee, API_FUNC);
  types::FunctionPointerType const *function_pointer(types::FunctionType const *functionType, API_FUNC);
  
  [[nodiscard("call the returned FunctionType object with arguments, e.g. function(...)(args...)")]]  
  impl::FunctionType function(types::TypeHandle ret, API_FUNC);
 
  [[nodiscard("call the returned StructType object with arguments, e.g. defineStruct(...)(args...)")]]
  impl::StructType defineStruct(std::string const &name, API_FUNC);  

}  

#include "typesystem_private.tpp"
