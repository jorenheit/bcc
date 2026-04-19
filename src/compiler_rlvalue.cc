#include "compiler.ih"

values::RValue Compiler::rValue(values::RValue const &val, API_CTX) const {
  (void)API_CTX_NAME;
  return values::RValue{val};
}

values::RValue Compiler::rValue(std::string const &var, API_CTX) const {
  API_REQUIRE_IN_SCOPE(var);
  return values::RValue{local(var)};
}

values::RValue Compiler::rValue(SlotProxy const &slot, API_CTX) const {
  (void)API_CTX_NAME;
  return values::RValue{slot};
}

values::RValue Compiler::rValue(values::Literal const &val, API_CTX) const {
  return values::RValue{val};
}

values::LValue Compiler::lValue(values::LValue const &val, API_CTX) const {
  (void)API_CTX_NAME;
  return values::LValue{val};
}

values::LValue Compiler::lValue(std::string const &var, API_CTX) const {
  API_REQUIRE_IN_SCOPE(var);
  return values::LValue{local(var)};
}
  
values::LValue Compiler::lValue(SlotProxy const &slot, API_CTX) const {
  (void)API_CTX_NAME;
  return values::LValue{slot};
}
