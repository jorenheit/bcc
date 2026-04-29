#include "assembler.ih"

Expression Assembler::rValue(Expression const &val, API_CTX) const {
  (void)API_CTX_NAME;
  return Expression{val};
}

Expression Assembler::rValue(std::string const &var, API_CTX) const {
  API_REQUIRE_IN_SCOPE(var);
  return Expression{local(var)};
}

Expression Assembler::rValue(SlotProxy const &slot, API_CTX) const {
  (void)API_CTX_NAME;
  return Expression{slot};
}

Expression Assembler::rValue(literal::Literal const &val, API_CTX) const {
  return Expression{val};
}

Expression Assembler::lValue(Expression const &val, API_CTX) const {
  API_REQUIRE(val.hasSlot(), "cannot convert expression '", val.str(), "' to L-value.");
  return Expression{val};
}

Expression Assembler::lValue(std::string const &var, API_CTX) const {
  API_REQUIRE_IN_SCOPE(var);
  return Expression{local(var)};
}
  
Expression Assembler::lValue(SlotProxy const &slot, API_CTX) const {
  (void)API_CTX_NAME;
  return Expression{slot};
}
