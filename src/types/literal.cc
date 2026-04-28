#include "acus/types/literal.h"

#define API_SOURCE
#include "acus/api/api.h"

namespace acus::literal {

  Literal i8(int val, API_FUNC) {
    API_FUNC_BEGIN_FREE();
    return std::make_shared<impl::i8>(val, API_FWD);
  }

  Literal i16(int val, API_FUNC) {
    API_FUNC_BEGIN_FREE();
    return std::make_shared<impl::i16>(val, API_FWD);
  }

  Literal string(std::string const &str, API_FUNC) {
    API_FUNC_BEGIN_FREE();
    return std::make_shared<impl::string>(str, API_FWD);
  }


  [[nodiscard("call the returned StructLiteral object with arguments, e.g. structT(...)(args...)")]]
  impl::StructLiteral structT(types::TypeHandle structType, API_FUNC) {
    API_FUNC_BEGIN_FREE();
    return impl::StructLiteral { structType, API_FWD };
  }

  [[nodiscard("call the returned arrayLiteral object with arguments, e.g. array(i8)(args...)")]]  
  impl::ArrayLiteral array(types::TypeHandle elementType, API_FUNC) {
    API_FUNC_BEGIN_FREE();
    return impl::ArrayLiteral { elementType, API_FWD };
  }
  
  Literal function_pointer(types::FunctionType const *fType, std::string const &fName, API_FUNC) {
    API_FUNC_BEGIN_FREE();
    return std::make_shared<impl::FunctionPointer>(fType, fName, API_FWD);
  }

} // namespace acus::literal
