#include "assembler.ih"

template <typename SpecType>
Expression Assembler::binOpAssignImpl(Expression const &lhs, Expression const &rhs, SpecType const &spec, API_CTX) {
  assert(not lhs.isLiteral());

  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_FUNCTION_BLOCK();
  auto opResult = types::rules::binOpResult(spec.op, lhs.type(), rhs.type());
  API_REQUIRE(opResult, error::ErrorCode::IncompatibleOperands, opResult.errorMsg);

  pushPtr();

  int stride = 1;
  Slot const lhsBase = lhs.slot()->materialize(*this);
  Slot targetSlot = lhsBase;
  if (types::isPointer(lhs.type())) {
    targetSlot = lhsBase.sub(ts::u16(), RuntimePointer::Offset);
    stride = types::cast<types::PointerType>(lhs.type())->pointeeType()->size();
  }
  
  if (stride != 1) {
    assert(spec.op == BinOp::Add || spec.op == BinOp::Sub);
  }

  if (rhs.hasSlot()) {
    Slot operandSlot = rhs.slot()->materialize(*this);
    bool freeOperandSlot = not rhs.slot()->direct();
    if (stride != 1) {
      // rhs operand needs to be multiplied, so we need a copy unless
      // this was an indirect access, in which case we can simply
      // modify the temporary materialized slot.
      
      if (rhs.slot()->direct()) {
	// Direct -> we need a temp copy
	Slot const operandCopy = getTemp(operandSlot.type);
	assignSlot(operandCopy, operandSlot);
	mulSlotByConst(operandCopy, stride);
	operandSlot = operandCopy;
	freeOperandSlot = true;
      } else {
	// Indirect -> we can can modify the slot 
	mulSlotByConst(operandSlot, stride);
      }
    }
    (this->*spec.applyWithSlot)(targetSlot, operandSlot);
    if (freeOperandSlot) freeTemp(operandSlot);

  }
  else {
    int const delta = literal::cast<types::IntegerType>(rhs.literal())->semanticValue();
    (this->*spec.applyWithConst)(targetSlot, stride * delta);
  }

  
  if (not lhs.slot()->direct()) {
    lhs.slot()->write(*this, lhsBase);
    freeTemp(lhsBase);
  }

  popPtr();
  return lhs;
}

template <typename SpecType>
Expression Assembler::binOpImpl(Expression const &lhs, Expression const &rhs, SpecType const &spec, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_FUNCTION_BLOCK();

  auto opResult = types::rules::binOpResult(spec.op, lhs.type(), rhs.type());
  API_REQUIRE(opResult, error::ErrorCode::IncompatibleOperands, opResult.errorMsg);

  if (lhs.isLiteral() && rhs.isLiteral()) {
    assert(types::isInteger(lhs.type()) && types::isInteger(rhs.type()));
    
    int const x = literal::cast<types::IntegerType>(lhs.literal())->semanticValue();
    int const y = literal::cast<types::IntegerType>(rhs.literal())->semanticValue();
    auto const result = spec.fold(x, y);

    if (types::isU8(opResult.type)) return Expression{literal::u8(result)};
    if (types::isS8(opResult.type)) return Expression{literal::s8(result)};
    if (types::isU16(opResult.type)) return Expression{literal::u16(result)};
    if (types::isS16(opResult.type)) return Expression{literal::s16(result)};

    std::unreachable();
  }

  Slot result = getTemp(opResult.workType);
  assignImpl(Expression{result}, lhs, API_FWD); // TODO: this should be assignSlot
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
