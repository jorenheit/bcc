#include "compiler.ih"

template <typename SpecType>
ExpressionResult Compiler::opAssignImpl(ExpressionResult const &lhs, ExpressionResult const &rhs, SpecType const &spec, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_BINOP(spec.op, lhs.type(), rhs.type());
  assert(not lhs.isLiteral());

  pushPtr();

  int stride = 1;
  Slot const lhsBase = lhs.slot()->materialize(*this);
  Slot targetSlot = lhsBase;
  if (types::isPointer(lhs.type())) {
    targetSlot = lhsBase.sub(TypeSystem::i16(), RuntimePointer::Offset);
    stride = types::cast<types::PointerType>(lhs.type())->pointeeType()->size();
  }
  
  if (stride != 1) {
    assert(spec.op == BinOp::Add || spec.op == BinOp::Sub);
  }

  if (rhs.hasSlot()) {
    Slot operandSlot = rhs.slot()->materialize(*this);
    if (stride != 1) {
      Slot const operandCopy = getTemp(operandSlot.type);
      assignSlot(operandCopy, operandSlot);
      mulSlotByConst(operandCopy, stride);
      operandSlot = operandCopy;
    }
    (this->*spec.applyWithSlot)(targetSlot, operandSlot);
  }
  else {
    int const delta = values::cast<types::IntegerType>(rhs.literal())->value();
    (this->*spec.applyWithConst)(targetSlot, stride * delta);
  }
  
  if (not lhs.slot()->direct()) {
    lhs.slot()->write(*this, lhsBase);
  }
  
  popPtr();

  return lhs;
}

template <typename SpecType>
ExpressionResult Compiler::opImpl(ExpressionResult const &lhs, ExpressionResult const &rhs, SpecType const &spec, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_BINOP(spec.op, lhs.type(), rhs.type());

  auto opResult = types::rules::binOpResult(spec.op, lhs.type(), rhs.type());
  assert(opResult);

  if (lhs.isLiteral() && rhs.isLiteral()) {
    int const x = values::cast<types::IntegerType>(lhs.literal())->value();
    int const y = values::cast<types::IntegerType>(rhs.literal())->value();
    auto const result = spec.fold(x, y);
    if (types::isI8(opResult.type))  return ExpressionResult{values::i8(result)};
    if (types::isI16(opResult.type)) return ExpressionResult{values::i16(result)};
    std::unreachable();
  }

  Slot result = getTemp(lhs.type());
  assignImpl(ExpressionResult{result}, lhs, API_FWD);
  opAssignImpl(ExpressionResult{result}, rhs, spec, API_FWD);

  result.type = opResult.type;
  return ExpressionResult{result};
}

// Explicit instantiations for Mop, Cop and Lop
template ExpressionResult Compiler::opImpl<Compiler::Mop>(ExpressionResult const&, ExpressionResult const&, Compiler::Mop const&, API_CTX);
template ExpressionResult Compiler::opImpl<Compiler::Lop>(ExpressionResult const&, ExpressionResult const&, Compiler::Lop const&, API_CTX);
template ExpressionResult Compiler::opImpl<Compiler::Cop>(ExpressionResult const&, ExpressionResult const&, Compiler::Cop const&, API_CTX);

template ExpressionResult Compiler::opAssignImpl<Compiler::Mop>(ExpressionResult const&, ExpressionResult const&, Compiler::Mop const&, API_CTX);
template ExpressionResult Compiler::opAssignImpl<Compiler::Lop>(ExpressionResult const&, ExpressionResult const&, Compiler::Lop const&, API_CTX);
template ExpressionResult Compiler::opAssignImpl<Compiler::Cop>(ExpressionResult const&, ExpressionResult const&, Compiler::Cop const&, API_CTX);
