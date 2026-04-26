#include "builder.ih"

Expression Builder::rValue(Expression const &val, API_CTX) const {
  (void)API_CTX_NAME;
  return Expression{val};
}

Expression Builder::rValue(std::string const &var, API_CTX) const {
  API_REQUIRE_IN_SCOPE(var);
  return Expression{local(var)};
}

Expression Builder::rValue(SlotProxy const &slot, API_CTX) const {
  (void)API_CTX_NAME;
  return Expression{slot};
}

Expression Builder::rValue(values::Literal const &val, API_CTX) const {
  return Expression{val};
}

Expression Builder::lValue(Expression const &val, API_CTX) const {
  API_REQUIRE(val.hasSlot(), "cannot convert expression '", val.str(), "' to L-value.");
  return Expression{val};
}

Expression Builder::lValue(std::string const &var, API_CTX) const {
  API_REQUIRE_IN_SCOPE(var);
  return Expression{local(var)};
}
  
Expression Builder::lValue(SlotProxy const &slot, API_CTX) const {
  (void)API_CTX_NAME;
  return Expression{slot};
}
