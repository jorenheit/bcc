#include "acus/api/error.h"
#include "acus/types/type_rules.h"
#include "acus/types/types.h"
#include "acus/api/context.h"

#ifndef API_LOC
#define API_LOC api_loc
#endif

#if !defined(API_HEADER) && !defined(API_SOURCE)
#define API_HEADER
#endif

#ifdef API_FUNC
#undef API_FUNC
#endif

#ifdef API_HEADER
#define API_FUNC API_FUNC_HEADER
#undef API_HEADER
#elif defined(API_SOURCE)
#define API_FUNC API_FUNC_SOURCE
#undef API_SOURCE
#else
#error __FILE__ ": define API_HEADER or API_SOURCE before including this file"
#endif

#ifndef API_H_INCLUDED
#define API_H_INCLUDED

namespace acus::api {
  struct ExpResult {
    bool expected;
    std::string msg;
  };
  
  void expectNext(std::string const &name);
  ExpResult isExpected(std::string const &name, bool strict = false);
  std::string expected();
  void clearExpected();
}

#define API_FUNC_HEADER std::source_location API_LOC = std::source_location::current()
#define API_FUNC_SOURCE std::source_location API_LOC
#define API_CTX_NAME api_ctx
#define API_CTX api::Context const & API_CTX_NAME
#define API_FWD API_CTX_NAME

#define API_FUNC_BEGIN()				\
  api::Context API_CTX_NAME{*this, __func__, API_LOC};

#define API_EXPECT_NEXT(name) api::expectNext((name));			

#define API_CHECK_EXPECTED_IMPL(strict) do {				\
    std::string const apiName = (API_CTX_NAME).apiName();		\
    auto expResult = api::isExpected(apiName, strict);			\
    error::throw_if(not expResult.expected,				\
		    (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		    expResult.msg);					\
    if (api::expected() == apiName) api::clearExpected();		\
  } while (false);

#define API_CHECK_EXPECTED() API_CHECK_EXPECTED_IMPL(false);
#define API_CHECK_EXPECTED_STRICT() API_CHECK_EXPECTED_IMPL(true);
#define API_CLEAR_EXPECTED() api::clearExpected();

#define API_REQUIRE(condition, ...)					\
  error::throw_if(not (condition),					\
		  (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		  __VA_ARGS__);

#define API_REQUIRE_OUTSIDE_PROGRAM_BLOCK()				\
  error::throw_if((API_CTX_NAME).programStarted(),			\
		  (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		  "called '", (API_CTX_NAME).apiName(), "' while inside a program-block.")

#define API_REQUIRE_INSIDE_PROGRAM_BLOCK()				\
  error::throw_if(not (API_CTX_NAME).programStarted(),			\
		  (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		  "called '", (API_CTX_NAME).apiName(), "' outside a program-block.")

#define API_REQUIRE_OUTSIDE_FUNCTION_BLOCK()				\
  error::throw_if(not (API_CTX_NAME).currentFunction().empty(),		\
		  (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		  "called '", (API_CTX_NAME).apiName(), "' while inside function-block '", \
		  (API_CTX_NAME).currentFunction(), "'; this operation must be used outside a function-block.")

#define API_REQUIRE_INSIDE_FUNCTION_BLOCK()				\
  error::throw_if((API_CTX_NAME).currentFunction().empty(),		\
		  (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		  "called '", (API_CTX_NAME).apiName(), "' outside a function-block.")

#define API_REQUIRE_OUTSIDE_CODE_BLOCK()				\
  error::throw_if(not (API_CTX_NAME).currentBlock().empty(),	\
		  (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		  "called '", (API_CTX_NAME).apiName(), "' while inside code-block '", \
		  (API_CTX_NAME).currentBlock(), "'; call 'endBlock' first.")

#define API_REQUIRE_INSIDE_CODE_BLOCK()					\
  error::throw_if((API_CTX_NAME).currentBlock().empty(),		\
		  (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		  "called '", (API_CTX_NAME).apiName(), "' outside a code-block.")

#define API_REQUIRE_NO_SCOPE()						\
  error::throw_if((API_CTX_NAME).currentScopeDepth() > 0,		\
		  (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		  "called '", (API_CTX_NAME).apiName(), "' while inside a scope-block at depth ", \
		  (API_CTX_NAME).currentScopeDepth(), "; call 'endScope' first until the scope depth returns to 0.")

#define API_REQUIRE_DECLARE_GLOBAL_ALLOWED()				\
  error::throw_if(not (API_CTX_NAME).globalDeclarationsAllowed(),	\
		  (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		  "called '", (API_CTX_NAME).apiName(), "' in a context where global declarations are not allowed.")

#define API_REQUIRE_GLOBAL_NAME_AVAILABLE(name)				\
  error::throw_if((API_CTX_NAME).declaredAsGlobal(name),		\
		  (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		  "cannot declare global '", name, "' because that name is already in use.")

#define API_REQUIRE_IS_GLOBAL(name)					\
  error::throw_if(not (API_CTX_NAME).declaredAsGlobal(name),		\
		  (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		  "'", name, "' is not a declared global.")

#define API_REQUIRE_IN_SCOPE(name)					\
  error::throw_if(not (API_CTX_NAME).inScope(name),			\
		  (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		  "'", name, "' is not declared in this scope.")

#define API_REQUIRE_NOT_IN_SCOPE(name)					\
  error::throw_if((API_CTX_NAME).inScope(name),		\
		  (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		  "'", name, "' is already declared in this scope.")

#define API_REQUIRE_NOT_IN_CURRENT_SCOPE(name)					\
  error::throw_if((API_CTX_NAME).inCurrentScope(name),		\
		  (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		  "'", name, "' is already declared in this scope.")

#define API_REQUIRE_IS_INTEGER(obj)					\
  error::throw_if(not types::isInteger((obj).type()),			\
		  (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		  "expected an integer in call to '", (API_CTX_NAME).apiName(), "', but got '", \
		  (obj).str(), "' of type '", (obj).type()->str(), "'.")

#define API_REQUIRE_IS_ARRAY(obj)					\
  error::throw_if(not types::isArray((obj).type()),			\
		  (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		  "expected an array in call to '", (API_CTX_NAME).apiName(), "', but got '", \
		  (obj).str(), "' of type '", (obj).type()->str(), "'.")

#define API_REQUIRE_IS_STRING(obj)					\
  error::throw_if(not types::isString((obj).type()),			\
		  (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		  "expected a string in call to '", (API_CTX_NAME).apiName(), "', but got '", \
		  (obj).str(), "' of type '", (obj).type()->str(), "'.")

#define API_REQUIRE_IS_ARRAY_OR_STRING(obj)				\
  error::throw_if(not (types::isArray((obj).type()) || types::isString((obj).type())), \
		  (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		  "expected an array or string in call to '", (API_CTX_NAME).apiName(), "', but got '", \
		  (obj).str(), "' of type '", (obj).type()->str(), "'.")

#define API_REQUIRE_IS_STRUCT(obj)					\
  error::throw_if(not types::isStruct((obj).type()),			\
		  (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		  "expected a struct in call to '", (API_CTX_NAME).apiName(), "', but got '", \
		  (obj).str(), "' of type '", (obj).type()->str(), "'.")

#define API_REQUIRE_IS_POINTER(obj)					\
  error::throw_if(not types::isPointer((obj).type()),			\
		  (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		  "expected a pointer in call to '", (API_CTX_NAME).apiName(), "', but got '", \
		  (obj).str(), "' of type '", (obj).type()->str(), "'.")

#define API_REQUIRE_IS_FUNCTION(fType)					\
  error::throw_if(not types::isFunction((fType)),			\
		  (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		  "expected a function-type in call to '", (API_CTX_NAME).apiName(), "', but got '", \
		  #fType, "' of type '", (fType)->str(), "'.")

#define API_REQUIRE_IS_FUNCTION_POINTER(obj)					\
  error::throw_if(not types::isFunctionPointer((obj).type()),		\
		  (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		  "expected a function-pointer in call to '", (API_CTX_NAME).apiName(), "', but got '", \
		  (obj).str(), "' of type '", (obj).type()->str(), "'.")

#define API_REQUIRE_FIELD_INDEX_IN_BOUNDS(obj, fieldIndex)		\
  error::throw_if((fieldIndex) < 0 ||					\
		  (fieldIndex) >= types::cast<types::StructType>((obj).type())->fieldCount(), \
		  (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		  "field index ", fieldIndex, " is out of bounds for struct '", \
		  (obj).type()->str(), "' with ",			\
		  types::cast<types::StructType>((obj).type())->fieldCount(), " fields.")

#define API_REQUIRE_IS_FIELD(obj, fieldName)				\
  error::throw_if(-1 == types::cast<types::StructType>((obj).type())->fieldIndex(fieldName), \
		  (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		  "struct '", (obj).type()->str(), "' has no field named '", \
		  fieldName, "'.")

#define API_REQUIRE_INDEX_IN_BOUNDS(arr, index)				\
  error::throw_if((index) < 0 ||					\
		  (index) >= types::cast<types::ArrayLike>((arr).type())->length(), \
		  (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		  "index ", index, " is out of bounds for '",		\
		  (arr).str(), "' of type '", (arr).type()->str(),	\
		  "' with length ",					\
		  types::cast<types::ArrayLike>((arr).type())->length(), ".")

#define API_REQUIRE_ASSIGNABLE(dest, src) do {				\
    auto result = types::rules::assignResult((dest), (src));		\
      error::throw_if(not result,					\
		      (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		      result.errorMsg);					\
  } while (false);


#define API_REQUIRE_BINOP(op, lhs, rhs) do {				\
    auto result = types::rules::binOpResult((op), (lhs), (rhs));		\
      error::throw_if(not result,					\
		      (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		      result.errorMsg);					\
  } while (false);

#define API_EXPECT_TYPE(t, expectedT) do {				\
    error::throw_if(t != expectedT,					\
		    (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		    "expected type '" + (expectedT)->str() + "', got '" + (t)->str() + "'"); \
  } while (false);

#define API_REQUIRE_PARAM_COUNT_MATCHES_FUNCTION(funcType, params) do { \
  error::throw_if(funcType->paramTypes().size() != (params).size(),		\
		  (API_CTX_NAME).file_name(), (API_CTX_NAME).line(), (API_CTX_NAME).column(), \
		  "number or parameters does not match function-type; expected ", \
		  funcType->paramTypes().size(), ", got ", (params).size(), "."); \
  } while (false);

		  

#endif // API_H
