
Compiler::FunctionCall Compiler::callFunction(std::string const &functionName, std::string const &nextBlockName,
						     auto const &returnSlot, API_FUNC_SOURCE) {
  API_FUNC_BEGIN();
  return FunctionCall { *this, functionName, nextBlockName,
			lValue(returnSlot, API_FWD), API_FWD };
}

void Compiler::returnFromFunction(auto const &rhs, API_FUNC_SOURCE) {
  API_FUNC_BEGIN();
  returnFromFunctionImpl(rValue(rhs, API_FWD), API_FWD);
}

void Compiler::writeOut(auto const &rhs, API_FUNC_SOURCE) {
  API_FUNC_BEGIN();
  writeOutImpl(rValue(rhs, API_FWD), API_FWD);
}

ExpressionResult Compiler::assign(auto const &lhs, auto const &rhs, API_FUNC_SOURCE) {
  API_FUNC_BEGIN();
  return assignImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
}

ExpressionResult Compiler::structField(auto const &obj, std::string const &field, API_FUNC_SOURCE) {
  API_FUNC_BEGIN();
  return structFieldImpl(rValue(obj, API_FWD), field, API_FWD);
}

ExpressionResult Compiler::structField(auto const &obj, int fieldIndex, API_FUNC_SOURCE) {
  API_FUNC_BEGIN();
  return structFieldImpl(rValue(obj, API_FWD), fieldIndex, API_FWD);
}

ExpressionResult Compiler::arrayElement(auto const &arr, int index, API_FUNC_SOURCE) {
  API_FUNC_BEGIN();
  return arrayElementImpl(rValue(arr, API_FWD), index, API_FWD);
}

ExpressionResult Compiler::arrayElement(auto const &arr, auto const &index, API_FUNC_SOURCE) {
  API_FUNC_BEGIN();
  return arrayElementImpl(rValue(arr, API_FWD), rValue(index, API_FWD), API_FWD);
}

ExpressionResult Compiler::dereferencePointer(auto const &ptr, API_FUNC_SOURCE) {
  API_FUNC_BEGIN();
  return dereferencePointerImpl(lValue(ptr, API_FWD), API_FWD);
}

ExpressionResult Compiler::addressOf(auto const &obj, API_FUNC_SOURCE) {
  API_FUNC_BEGIN();
  return addressOfImpl(lValue(obj, API_FWD), API_FWD);
}

void Compiler::branchIf(auto const &condition, std::string const &trueLabel, std::string const &falseLabel, API_FUNC_SOURCE) {
  API_FUNC_BEGIN();
  return branchIfImpl(rValue(condition, API_FWD), trueLabel, falseLabel, API_FWD);
}

// Binary operations
// The implementation for opImpl and opAssignImpl is in compiler_binop_general.cc

#define BINOP(OP)							\
  ExpressionResult Compiler::OP##Assign(auto const &lhs, auto const &rhs, API_FUNC_SOURCE) { \
    API_FUNC_BEGIN();							\
    return opAssignImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), OP##Spec, API_FWD); \
  }									\
									\
  ExpressionResult Compiler::OP(auto const &lhs, auto const &rhs, API_FUNC_SOURCE) { \
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

