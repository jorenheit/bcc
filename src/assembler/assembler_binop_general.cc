#include "assembler.ih"

template <typename SpecType>
Expression Assembler::binOpAssignImpl(Expression const &lhs, Expression const &rhs, SpecType const &spec, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_BINOP(spec.op, lhs.type(), rhs.type());
  assert(not lhs.isLiteral());

  pushPtr();

  int stride = 1;
  Slot const lhsBase = lhs.slot()->materialize(*this);
  Slot targetSlot = lhsBase;
  if (types::isPointer(lhs.type())) {
    targetSlot = lhsBase.sub(ts::i16(), RuntimePointer::Offset);
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
    int const delta = literal::cast<types::IntegerType>(rhs.literal())->value();
    (this->*spec.applyWithConst)(targetSlot, stride * delta);
  }
  
  if (not lhs.slot()->direct()) {
    lhs.slot()->write(*this, lhsBase);
  }
  
  popPtr();

  return lhs;
}

template <typename SpecType>
Expression Assembler::binOpImpl(Expression const &lhs, Expression const &rhs, SpecType const &spec, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_BINOP(spec.op, lhs.type(), rhs.type());

  auto opResult = types::rules::binOpResult(spec.op, lhs.type(), rhs.type());
  assert(opResult);

  if (lhs.isLiteral() && rhs.isLiteral()) {
    int const x = literal::cast<types::IntegerType>(lhs.literal())->value();
    int const y = literal::cast<types::IntegerType>(rhs.literal())->value();
    auto const result = spec.fold(x, y);
    if (types::isI8(opResult.type))  return Expression{literal::i8(result)};
    if (types::isI16(opResult.type)) return Expression{literal::i16(result)};
    std::unreachable();
  }

  Slot result = getTemp(lhs.type());
  assignImpl(Expression{result}, lhs, API_FWD);
  binOpAssignImpl(Expression{result}, rhs, spec, API_FWD);

  result.type = opResult.type;
  return Expression{result};
}

// Explicit instantiations for Mop, Cop and Lop
template Expression Assembler::binOpImpl<Assembler::Mop>(Expression const&, Expression const&, Assembler::Mop const&, API_CTX);
template Expression Assembler::binOpImpl<Assembler::Lop>(Expression const&, Expression const&, Assembler::Lop const&, API_CTX);
template Expression Assembler::binOpImpl<Assembler::Cop>(Expression const&, Expression const&, Assembler::Cop const&, API_CTX);

template Expression Assembler::binOpAssignImpl<Assembler::Mop>(Expression const&, Expression const&, Assembler::Mop const&, API_CTX);
template Expression Assembler::binOpAssignImpl<Assembler::Lop>(Expression const&, Expression const&, Assembler::Lop const&, API_CTX);
template Expression Assembler::binOpAssignImpl<Assembler::Cop>(Expression const&, Expression const&, Assembler::Cop const&, API_CTX);
