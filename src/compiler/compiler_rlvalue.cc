#include "compiler.ih"

ExpressionResult Compiler::rValue(ExpressionResult const &val, API_CTX) const {
  (void)API_CTX_NAME;
  return ExpressionResult{val};
}

ExpressionResult Compiler::rValue(std::string const &var, API_CTX) const {
  API_REQUIRE_IN_SCOPE(var);
  return ExpressionResult{local(var)};
}

ExpressionResult Compiler::rValue(SlotProxy const &slot, API_CTX) const {
  (void)API_CTX_NAME;
  return ExpressionResult{slot};
}

ExpressionResult Compiler::rValue(values::Literal const &val, API_CTX) const {
  return ExpressionResult{val};
}

ExpressionResult Compiler::lValue(ExpressionResult const &val, API_CTX) const {
  API_REQUIRE(val.hasSlot(), "cannot convert expression '", val.str(), "' to L-value.");
  return ExpressionResult{val};
}

ExpressionResult Compiler::lValue(std::string const &var, API_CTX) const {
  API_REQUIRE_IN_SCOPE(var);
  return ExpressionResult{local(var)};
}
  
ExpressionResult Compiler::lValue(SlotProxy const &slot, API_CTX) const {
  (void)API_CTX_NAME;
  return ExpressionResult{slot};
}
