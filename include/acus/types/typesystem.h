#pragma once
#include "acus/types/types_fwd.h"

#define API_HEADER
#include "acus/api/api.h"

// Public typesystem API (implemented in typesystem.cc)
namespace acus::ts {
  struct FunctionTypeBuilder;
  struct StructTypeBuilder;  

  void init();

  types::VoidType const *void_t(API_FUNC);
  types::IntegerType const *i8(API_FUNC);
  types::IntegerType const *i16(API_FUNC);
  types::ArrayType const *array(types::TypeHandle elem, int length, API_FUNC);
  types::StringType const *string(int maxLen, API_FUNC);    
  types::RawType const *raw(int n, API_FUNC);
  types::StructType const *struct_t(std::string const &name, API_FUNC);
  types::PointerType const *pointer(types::TypeHandle pointee, API_FUNC);
  types::FunctionPointerType const *function_pointer(types::FunctionType const *functionType, API_FUNC);
  types::FunctionType const *void_function(API_FUNC);

  FunctionTypeBuilder function(API_FUNC);
  StructTypeBuilder defineStruct(std::string const &name, API_FUNC);  
}

// Builder Classes (implemented in type_builders.cc)
namespace acus::ts {
  
  class FunctionTypeBuilder {
  public:
    FunctionTypeBuilder &ret(types::TypeHandle returnType);    
    FunctionTypeBuilder &param(types::TypeHandle paramType);
    FunctionTypeBuilder &param(std::vector<types::TypeHandle> const &paramTypes);    
    types::FunctionType const *done();
    
    FunctionTypeBuilder(api::impl::Context const &ctx);      
    ~FunctionTypeBuilder() noexcept(false);
    
  private:
    types::TypeHandle _ret = types::null;
    api::impl::Context API_CTX_NAME;
    bool _finalized = false;
    std::vector<types::TypeHandle> _paramTypes;
    
  
    FunctionTypeBuilder(FunctionTypeBuilder const&) = delete;
    FunctionTypeBuilder(FunctionTypeBuilder&&) = delete;
    FunctionTypeBuilder& operator=(FunctionTypeBuilder const&) = delete;
    FunctionTypeBuilder& operator=(FunctionTypeBuilder&&) = delete;
  };  

  class StructTypeBuilder {
  public:
    StructTypeBuilder(std::string const &structName, api::impl::Context const &ctx);
    StructTypeBuilder &field(std::string const &name, types::TypeHandle type);
    types::StructType const *done();
    ~StructTypeBuilder() noexcept(false);

    
  private:
    std::string _structName;
    api::impl::Context API_CTX_NAME;
    bool _finalized = false;

    using NameTypePair = std::pair<std::string, types::TypeHandle>;
    std::vector<types::NameTypePair> _fields;
    
    StructTypeBuilder(StructTypeBuilder const&) = delete;
    StructTypeBuilder(StructTypeBuilder&&) = delete;
    StructTypeBuilder& operator=(StructTypeBuilder const&) = delete;
    StructTypeBuilder& operator=(StructTypeBuilder&&) = delete;
  };

  
} // namespace impl
