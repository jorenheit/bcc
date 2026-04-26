#include "compiler.ih"

Expression Compiler::rValue(Expression const &val, API_CTX) const {
  (void)API_CTX_NAME;
  return Expression{val};
}

Expression Compiler::rValue(std::string const &var, API_CTX) const {
  API_REQUIRE_IN_SCOPE(var);
  return Expression{local(var)};
}

Expression Compiler::rValue(SlotProxy const &slot, API_CTX) const {
  (void)API_CTX_NAME;
  return Expression{slot};
}

Expression Compiler::rValue(values::Literal const &val, API_CTX) const {
  return Expression{val};
}

Expression Compiler::lValue(Expression const &val, API_CTX) const {
  API_REQUIRE(val.hasSlot(), "cannot convert expression '", val.str(), "' to L-value.");
  return Expression{val};
}

Expression Compiler::lValue(std::string const &var, API_CTX) const {
  API_REQUIRE_IN_SCOPE(var);
  return Expression{local(var)};
}
  
Expression Compiler::lValue(SlotProxy const &slot, API_CTX) const {
  (void)API_CTX_NAME;
  return Expression{slot};
}
