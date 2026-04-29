
Assembler::FunctionCallBuilder Assembler::callFunctionPointer(auto const &functionPointer, std::string const &nextBlockName, API_FUNC_SOURCE) {
  API_FUNC_BEGIN();
  return FunctionCallBuilder { *this, rValue(functionPointer, API_FWD), nextBlockName, API_FWD };
}

void Assembler::returnFromFunction(auto const &rhs, API_FUNC_SOURCE) {
  API_FUNC_BEGIN();
  returnFromFunctionImpl(rValue(rhs, API_FWD), API_FWD);
}

void Assembler::writeOut(auto const &rhs, API_FUNC_SOURCE) {
  API_FUNC_BEGIN();
  writeOutImpl(rValue(rhs, API_FWD), API_FWD);
}

Expression Assembler::assign(auto const &lhs, auto const &rhs, API_FUNC_SOURCE) {
  API_FUNC_BEGIN();
  return assignImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
}

Expression Assembler::structField(auto const &obj, std::string const &field, API_FUNC_SOURCE) {
  API_FUNC_BEGIN();
  return structFieldImpl(rValue(obj, API_FWD), field, API_FWD);
}

Expression Assembler::structField(auto const &obj, int fieldIndex, API_FUNC_SOURCE) {
  API_FUNC_BEGIN();
  return structFieldImpl(rValue(obj, API_FWD), fieldIndex, API_FWD);
}

Expression Assembler::arrayElement(auto const &arr, int index, API_FUNC_SOURCE) {
  API_FUNC_BEGIN();
  return arrayElementImpl(rValue(arr, API_FWD), index, API_FWD);
}

Expression Assembler::arrayElement(auto const &arr, auto const &index, API_FUNC_SOURCE) {
  API_FUNC_BEGIN();
  return arrayElementImpl(rValue(arr, API_FWD), rValue(index, API_FWD), API_FWD);
}

Expression Assembler::dereferencePointer(auto const &ptr, API_FUNC_SOURCE) {
  API_FUNC_BEGIN();
  return dereferencePointerImpl(lValue(ptr, API_FWD), API_FWD);
}

Expression Assembler::addressOf(auto const &obj, API_FUNC_SOURCE) {
  API_FUNC_BEGIN();
  return addressOfImpl(lValue(obj, API_FWD), API_FWD);
}

void Assembler::branchIf(auto const &condition, std::string const &trueLabel, std::string const &falseLabel, API_FUNC_SOURCE) {
  API_FUNC_BEGIN();
  return branchIfImpl(rValue(condition, API_FWD), trueLabel, falseLabel, API_FWD);
}



Expression Assembler::expr(auto const &obj, API_FUNC_SOURCE) {
  API_FUNC_BEGIN();
  return rValue(obj, API_FWD);
}

// Binary operations
Expression Assembler::binOp(BinOp op, auto const &lhs, auto const &rhs, API_FUNC_SOURCE) {
  API_FUNC_BEGIN();
  switch (op) {
  case BinOp::Add:  return add(lhs, rhs, API_FWD);
  case BinOp::Sub:  return sub(lhs, rhs, API_FWD);
  case BinOp::Mul:  return mul(lhs, rhs, API_FWD);
  case BinOp::Div:  return div(lhs, rhs, API_FWD);
  case BinOp::Mod:  return mod(lhs, rhs, API_FWD);
  case BinOp::And:  return land(lhs, rhs, API_FWD);
  case BinOp::Nand: return lnand(lhs, rhs, API_FWD);
  case BinOp::Or:   return lor(lhs, rhs, API_FWD);
  case BinOp::Nor:  return lnor(lhs, rhs, API_FWD);
  case BinOp::Xor:  return lxor(lhs, rhs, API_FWD);
  case BinOp::Xnor: return lxnor(lhs, rhs, API_FWD);
  case BinOp::Eq:   return eq(lhs, rhs, API_FWD);
  case BinOp::Neq:  return neq(lhs, rhs, API_FWD);
  case BinOp::Lt:   return lt(lhs, rhs, API_FWD);
  case BinOp::Le:   return le(lhs, rhs, API_FWD);
  case BinOp::Gt:   return gt(lhs, rhs, API_FWD);
  case BinOp::Ge:   return ge(lhs, rhs, API_FWD);
  default: std::unreachable();
  }
  std::unreachable();
}

Expression Assembler::binOpAssign(BinOp op, auto const &lhs, auto const &rhs, API_FUNC_SOURCE) {
  API_FUNC_BEGIN();
  switch (op) {
  case BinOp::Add:  return addAssign(lhs, rhs, API_FWD);
  case BinOp::Sub:  return subAssign(lhs, rhs, API_FWD);
  case BinOp::Mul:  return mulAssign(lhs, rhs, API_FWD);
  case BinOp::Div:  return divAssign(lhs, rhs, API_FWD);
  case BinOp::Mod:  return modAssign(lhs, rhs, API_FWD);
  case BinOp::And:  return landAssign(lhs, rhs, API_FWD);
  case BinOp::Nand: return lnandAssign(lhs, rhs, API_FWD);
  case BinOp::Or:   return lorAssign(lhs, rhs, API_FWD);
  case BinOp::Nor:  return lnorAssign(lhs, rhs, API_FWD);
  case BinOp::Xor:  return lxorAssign(lhs, rhs, API_FWD);
  case BinOp::Xnor: return lxnorAssign(lhs, rhs, API_FWD);
  case BinOp::Eq:   return eqAssign(lhs, rhs, API_FWD);
  case BinOp::Neq:  return neqAssign(lhs, rhs, API_FWD);
  case BinOp::Lt:   return ltAssign(lhs, rhs, API_FWD);
  case BinOp::Le:   return leAssign(lhs, rhs, API_FWD);
  case BinOp::Gt:   return gtAssign(lhs, rhs, API_FWD);
  case BinOp::Ge:   return geAssign(lhs, rhs, API_FWD);
  default: std::unreachable();
  }
  std::unreachable();
}

// The implementation for opImpl and opAssignImpl is in Builder_binop_general.cc

#define BINOP(OP)							\
  Expression Assembler::OP##Assign(auto const &lhs, auto const &rhs, API_FUNC_SOURCE) { \
    API_FUNC_BEGIN();							\
    return opAssignImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), OP##Spec, API_FWD); \
  }									\
									\
  Expression Assembler::OP(auto const &lhs, auto const &rhs, API_FUNC_SOURCE) { \
    API_FUNC_BEGIN();							\
    return opImpl(rValue(lhs, API_FWD), rValue(rhs, API_FWD), OP##Spec, API_FWD); \
  }

// Arithmetic
BINOP(add);
BINOP(sub);
BINOP(mul);
BINOP(div);
BINOP(mod);

// Logic
BINOP(land);
BINOP(lnand);
BINOP(lor);
BINOP(lnor);
BINOP(lxor);
BINOP(lxnor);

// Comparisons
BINOP(eq);
BINOP(neq);
BINOP(lt);
BINOP(le);
BINOP(gt);
BINOP(ge);

#undef BINOP

