#include "acus/types/typesystem.h"

#define API_SOURCE
#include "acus/api/api.h"
using namespace acus;

// TODO: rename typesystem.cc and move types namespace implementations here

namespace acus::ts::impl {
  
  std::unique_ptr<types::VoidType> _void;
  std::unique_ptr<types::IntegerType> _i8;
  std::unique_ptr<types::IntegerType> _i16;

  std::vector<std::unique_ptr<types::RawType>> _rawTypes;    
  std::vector<std::unique_ptr<types::ArrayType>> _arrayTypes;
  std::vector<std::unique_ptr<types::StringType>> _stringTypes;
  std::vector<std::unique_ptr<types::StructType>> _structTypes;
  std::vector<std::unique_ptr<types::PointerType>> _pointerTypes;
  std::vector<std::unique_ptr<types::FunctionType>> _functionTypes;
  std::vector<std::unique_ptr<types::FunctionPointerType>> _functionPointerTypes;

  // StructType Builder
  types::StructType const *StructType::operator()(std::vector<NameTypePair> const &fields) && {
    _called = true;  
    if (ts::structT(_structName) != nullptr) return nullptr; // already defined a struct with this name
    ts::impl::_structTypes.emplace_back(std::make_unique<types::StructType>(_structName, fields));
    return ts::impl::_structTypes.back().get();
  }

  StructType::StructType(std::string const &structName, api::impl::Context const &ctx):
    _structName(structName),
    API_CTX_NAME(ctx)
  {}

  StructType::~StructType() noexcept(false) {
    API_REQUIRE(_called, "operator() must be called on result of defineStruct(); e.g. defineStruct(\"Point\")(\"x\", i8, \"y\", i8);");
  }

  // FunctionType Builder  
  types::FunctionType const *FunctionType::operator()(std::vector<types::TypeHandle> const &paramTypes) && {
    _called = true;
    for (auto const &ptr: _functionTypes) {
      if (_ret == ptr->returnType()) {
	if (ptr->paramTypes().size() != paramTypes.size()) continue;

	bool match = true;
	if (paramTypes.size() == 0) break;
	for (size_t i = 0; i != paramTypes.size(); ++i) {
	  if (paramTypes[i] != ptr->paramTypes()[i]) {
	    match = false;
	    break;
	  }
	}
	if (match) return ptr.get();
      }
    }
    _functionTypes.emplace_back(std::make_unique<types::FunctionType>(_ret, paramTypes));
    return _functionTypes.back().get();
  }

  FunctionType::FunctionType(types::TypeHandle ret, api::impl::Context const &ctx):
    _ret(ret),
    API_CTX_NAME(ctx)
  {}

  FunctionType::~FunctionType() noexcept(false) {
    API_REQUIRE(_called, "operator() must be called on result of function(); e.g. function(i8)(i8).");
  }

} // namespace impl

void ts::init() {
  static bool initialized = false;
  if (!initialized) {
    impl::_void = std::make_unique<types::VoidType>();
    impl::_i8 = std::make_unique<types::IntegerType>(8);
    impl::_i16 = std::make_unique<types::IntegerType>(16);
    initialized = true;
  }
}

types::VoidType const *ts::voidT(API_FUNC) {
  API_FUNC_BEGIN_FREE();
  return impl::_void.get();
}
  
types::IntegerType const *ts::i8(API_FUNC) {
  API_FUNC_BEGIN_FREE();
  return impl::_i8.get();
}
  
types::IntegerType const *ts::i16(API_FUNC) {
  API_FUNC_BEGIN_FREE();
  return impl::_i16.get();
}

types::ArrayType const *ts::array(types::TypeHandle elem, int length, API_FUNC) {
  API_FUNC_BEGIN_FREE();

  for (auto const &ptr: impl::_arrayTypes) {
    if (ptr->elementType() == elem && ptr->length() == length) return ptr.get();
  }
  impl::_arrayTypes.emplace_back(std::make_unique<types::ArrayType>(elem, length));
  return impl::_arrayTypes.back().get();
}

types::StringType const *ts::string(int maxLen, API_FUNC) {
  API_FUNC_BEGIN_FREE();
    
  for (auto const &ptr: impl::_stringTypes) {
    if (ptr->size() == maxLen + 1) {
      return ptr.get();
    }
  }
  impl::_stringTypes.emplace_back(std::make_unique<types::StringType>(i8(), maxLen));
  return impl::_stringTypes.back().get();
}
    
types::RawType const *ts::raw(int n, API_FUNC) {
  API_FUNC_BEGIN_FREE();
    
  for (auto const &ptr: impl::_rawTypes) {
    if (ptr->size() == n) return ptr.get();
  }
  impl::_rawTypes.emplace_back(std::make_unique<types::RawType>(n));
  return impl::_rawTypes.back().get();
}


types::StructType const *ts::structT(std::string const &name, API_FUNC) {
  API_FUNC_BEGIN_FREE();
    
  for (auto const &s: impl::_structTypes) {
    if (s->str() == name) return s.get();
  }
  return nullptr; // no struct by this name exists
}
  
types::PointerType const *ts::pointer(types::TypeHandle pointee, API_FUNC) {
  API_FUNC_BEGIN_FREE();
    
  for (auto const &ptr: impl::_pointerTypes) {
    if (ptr->pointeeType() == pointee) {
      return ptr.get();
    }
  }
  impl::_pointerTypes.emplace_back(std::make_unique<types::PointerType>(pointee));
  return impl::_pointerTypes.back().get();
}

types::FunctionPointerType const *ts::function_pointer(types::FunctionType const *functionType, API_FUNC) {
  API_FUNC_BEGIN_FREE();
    
  for (auto const &ptr: impl::_functionPointerTypes) {
    if (ptr->functionType() == functionType) {
      return ptr.get();
    }
  }
  impl::_functionPointerTypes.emplace_back(std::make_unique<types::FunctionPointerType>(functionType));
  return impl::_functionPointerTypes.back().get();
}

ts::impl::FunctionType ts::function(types::TypeHandle ret, API_FUNC) {
  API_FUNC_BEGIN_FREE();
  return impl::FunctionType { ret, API_FWD };
}

ts::impl::StructType ts::defineStruct(std::string const &name, API_FUNC) {
  API_FUNC_BEGIN_FREE();
  return impl::StructType { name, API_FWD };
}



