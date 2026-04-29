#include "acus/types/typesystem.h"
#include <iostream> //debug
namespace acus::ts::impl {
  
  extern std::unique_ptr<types::VoidType> _void;
  extern std::unique_ptr<types::IntegerType> _i8;
  extern std::unique_ptr<types::IntegerType> _i16;
  extern std::unique_ptr<types::FunctionType> _voidFunction;
  extern std::vector<std::unique_ptr<types::RawType>> _rawTypes;    
  extern std::vector<std::unique_ptr<types::ArrayType>> _arrayTypes;
  extern std::vector<std::unique_ptr<types::StringType>> _stringTypes;
  extern std::vector<std::unique_ptr<types::StructType>> _structTypes;
  extern std::vector<std::unique_ptr<types::PointerType>> _pointerTypes;
  extern std::vector<std::unique_ptr<types::FunctionType>> _functionTypes;
  extern std::vector<std::unique_ptr<types::FunctionPointerType>> _functionPointerTypes;
}


// FunctionTypeBuilder
namespace acus::ts {

  FunctionTypeBuilder &FunctionTypeBuilder::ret(types::TypeHandle returnType) {
    API_REQUIRE(_ret == types::null, "return-type was already specified.");
    _ret = returnType;
    return *this;
  }
    
  FunctionTypeBuilder &FunctionTypeBuilder::param(types::TypeHandle paramType) {
    _paramTypes.push_back(paramType);
    return *this;
  }

  FunctionTypeBuilder &FunctionTypeBuilder::param(std::vector<types::TypeHandle> const &paramTypes) {
    for (auto t: paramTypes) _paramTypes.push_back(t);
    return *this;
  }

  types::FunctionType const *FunctionTypeBuilder::done() {
    _finalized = true;
    
    if (_ret == types::null) _ret = ts::void_t();
    if (_ret == ts::void_t() && _paramTypes.size() == 0) {
      return ts::void_function();
    }
    
    for (auto const &ptr: impl::_functionTypes) {
      if (_ret == ptr->returnType()) {
	if (ptr->paramTypes().size() != _paramTypes.size()) continue;

	bool match = true;
	if (_paramTypes.size() == 0) break;
	for (size_t i = 0; i != _paramTypes.size(); ++i) {
	  if (_paramTypes[i] != ptr->paramTypes()[i]) {
	    match = false;
	    break;
	  }
	}
	if (match) return ptr.get();
      }
    }
    impl::_functionTypes.emplace_back(std::make_unique<types::FunctionType>(_ret, _paramTypes));
    return impl::_functionTypes.back().get();
  }

  FunctionTypeBuilder::FunctionTypeBuilder(api::impl::Context const &ctx):
    _ret(types::null),
    API_CTX_NAME(ctx)
  {}

  FunctionTypeBuilder::~FunctionTypeBuilder() noexcept(false) {
    if (std::uncaught_exceptions() == 0)
      API_REQUIRE(_finalized, "done() was never called on result of ts::function()");
  }
}


// StructTypeBuilder
namespace acus::ts {

  StructTypeBuilder::StructTypeBuilder(std::string const &structName, api::impl::Context const &ctx):
    _structName(structName),
    API_CTX_NAME(ctx)
  {}

  StructTypeBuilder &StructTypeBuilder::field(std::string const &name, types::TypeHandle type) {
    _fields.emplace_back(name, type);
    return *this;
  }

  types::StructType const *StructTypeBuilder::done() {
    _finalized = true;
    if (struct_t(_structName) != nullptr) return nullptr; // already defined a struct with this name
    impl::_structTypes.emplace_back(std::make_unique<types::StructType>(_structName, _fields));
    return impl::_structTypes.back().get();
  }

  StructTypeBuilder::~StructTypeBuilder() noexcept(false) {
    if (std::uncaught_exceptions() == 0)
      API_REQUIRE(_finalized, "done() was never called on result of ts::structT()");
  }

}
