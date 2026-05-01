#include "assembler.ih"

// TODO: factor common structure

Expression Assembler::lnotImpl(Expression const &obj, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_IS_INTEGER(obj);

  if (obj.isLiteral()) {
    int const val = literal::cast<types::IntegerType>(obj.literal())->encodedValue();
    return Expression { literal::i8(!val) };
  }

  // Apply to temp copy
  Slot result = getTemp(obj.type());
  assignSlot(result, obj.slot()->materialize(*this));
  notSlot(result);
  result.type = ts::i8();
  return Expression { result };
}

Expression Assembler::lnotAssignImpl(Expression const &obj, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_IS_INTEGER(obj);
  assert(not obj.isLiteral());

  Slot const objSlot = obj.slot()->materialize(*this);
  notSlot(objSlot);
  if (not obj.slot()->direct()) {
    obj.slot()->write(*this, objSlot);
  }
  return obj;
}

Expression Assembler::lboolImpl(Expression const &obj, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_IS_INTEGER(obj);

  if (obj.isLiteral()) {
    int const val = literal::cast<types::IntegerType>(obj.literal())->encodedValue();
    return Expression { literal::i8(!!val) };
  }

  // Apply to temp copy
  Slot result = getTemp(obj.type());
  assignSlot(result, obj.slot()->materialize(*this));
  boolSlot(result);
  result.type = ts::i8();
  return Expression { result };
}

Expression Assembler::lboolAssignImpl(Expression const &obj, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_IS_INTEGER(obj);
  assert(not obj.isLiteral());

  Slot const objSlot = obj.slot()->materialize(*this);
  boolSlot(objSlot);
  if (not obj.slot()->direct()) {
    obj.slot()->write(*this, objSlot);
  }
  return obj;
}

Expression Assembler::negateImpl(Expression const &obj, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_IS_SIGNED_INTEGER(obj);

  if (obj.isLiteral()) {
    int const val = literal::cast<types::IntegerType>(obj.literal())->semanticValue();
    return obj.type()->usesValue1()
      ? Expression { literal::s16(-val) }
      : Expression { literal::s8(-val) };
  }

  // Apply to temp copy
  Slot result = getTemp(obj.type());
  assignSlot(result, obj.slot()->materialize(*this));
  negateSlot(result);
  return Expression { result };
}


Expression Assembler::negateAssignImpl(Expression const &obj, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_IS_SIGNED_INTEGER(obj);
  assert(not obj.isLiteral());

  Slot const objSlot = obj.slot()->materialize(*this);
  negateSlot(objSlot);
  if (not obj.slot()->direct()) {
    obj.slot()->write(*this, objSlot);
  }
  return obj;
}


Expression Assembler::absImpl(Expression const &obj, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_IS_INTEGER(obj);

  if (obj.isLiteral()) {
    if (types::isUnsignedInteger(obj.type())) return obj;
    int const val = literal::cast<types::IntegerType>(obj.literal())->semanticValue();
    return obj.type()->usesValue1()
      ? Expression { literal::s16(val < 0 ? -val : val) }
      : Expression { literal::s8(val < 0 ? -val : val) };
  } 

  // Apply to temp copy
  Slot result = getTemp(obj.type());
  assignSlot(result, obj.slot()->materialize(*this));
  absSlot(result);
  return Expression { result };
}


Expression Assembler::absAssignImpl(Expression const &obj, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_IS_INTEGER(obj);
  assert(not obj.isLiteral());

  Slot const objSlot = obj.slot()->materialize(*this);
  absSlot(objSlot);
  if (not obj.slot()->direct()) {
    obj.slot()->write(*this, objSlot);
  }
  return obj;
}


Expression Assembler::signBitImpl(Expression const &obj, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_IS_SIGNED_INTEGER(obj);

  if (obj.isLiteral()) {
    int const val = literal::cast<types::IntegerType>(obj.literal())->semanticValue();
    return obj.type()->usesValue1()
      ? Expression { literal::s16(val < 0) }
      : Expression { literal::s8(val < 0) };
  } 

  // Apply to temp copy
  Slot result = getTemp(obj.type());
  assignSlot(result, obj.slot()->materialize(*this));
  signBitSlot(result);
  return Expression { result };
}


Expression Assembler::signBitAssignImpl(Expression const &obj, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_IS_SIGNED_INTEGER(obj);
  assert(not obj.isLiteral());

  Slot const objSlot = obj.slot()->materialize(*this);
  signBitSlot(objSlot);
  if (not obj.slot()->direct()) {
    obj.slot()->write(*this, objSlot);
  }
  return obj;
}



void Assembler::notSlot(Slot const &rhs) {
  assert(rhs.size() == 1);

  pushPtr();
  moveTo(rhs);

  if (rhs.type->usesValue1()) {
    not16Destructive(Cell{rhs, MacroCell::Value1},
		     Temps<1>::select(rhs, MacroCell::Scratch0));
  } else {
    notDestructive(Temps<1>::select(rhs, MacroCell::Scratch0));
  }
  
  popPtr();
}


void Assembler::boolSlot(Slot const &rhs) {
  assert(rhs.size() == 1);
  
  pushPtr();
  moveTo(rhs);

  if (rhs.type->usesValue1()) {
    bool16Destructive(Cell{rhs, MacroCell::Value1},
		     Temps<1>::select(rhs, MacroCell::Scratch0));
  } else {
    boolDestructive(Temps<1>::select(rhs, MacroCell::Scratch0));
  }
  
  popPtr();
}

void Assembler::negateSlot(Slot const &rhs) {
  assert(rhs.size() == 1);

  pushPtr();

  moveTo(rhs, MacroCell::Value0);
  if (rhs.type->usesValue1()) {
    Slot const tmp = getTemp(ts::raw(1));
    negate16Destructive(Cell{rhs, MacroCell::Value1},
			Temps<6>::select(rhs, MacroCell::Scratch0,
					 rhs, MacroCell::Scratch1,
					 rhs, MacroCell::Payload0,
					 rhs, MacroCell::Payload1,
					 tmp, MacroCell::Scratch0,
					 tmp, MacroCell::Scratch1));
  } else {
    negateDestructive(Temps<2>::select(rhs, MacroCell::Scratch0,
				       rhs, MacroCell::Scratch1));
  }

  popPtr();
}

void Assembler::absSlot(Slot const &rhs) {
  assert(types::isInteger(rhs.type));

  if (types::isUnsignedInteger(rhs.type)) return;

  pushPtr();
  // Construct the signbit in rhs.Flag
  moveTo(rhs, rhs.type->usesValue1() ? MacroCell::Value1 : MacroCell::Value0);
  signBitConstructive(Cell{rhs, MacroCell::Flag},
		      Temps<4>::select(rhs, MacroCell::Scratch0, rhs, MacroCell::Scratch1,
				       rhs, MacroCell::Payload0, rhs, MacroCell::Payload1));

  // If the sign-bit was set, negate the slot
  moveTo(rhs, MacroCell::Flag);
  loopOpen(); {
    zeroCell();
    negateSlot(rhs);
  } loopClose();

  popPtr();
}

void Assembler::signBitSlot(Slot const &rhs) {
  assert(types::isSignedInteger(rhs.type));
  
  pushPtr();
  moveTo(rhs, rhs.type->usesValue1() ? MacroCell::Value1 : MacroCell::Value0);
  signBitDestructive(Temps<3>::select(rhs, MacroCell::Scratch0,
				      rhs, MacroCell::Scratch1,
				      rhs, MacroCell::Payload0));
  popPtr();
}

void Assembler::signBitDestructive(Temps<3> tmp) {
  Cell const current = _dp.current();
  Cell const oneTwentyEight = tmp.get<0>();

  pushPtr();
  moveTo(oneTwentyEight);
  setToValue(128);
  moveTo(current);
  greaterOrEqualDestructive(oneTwentyEight, tmp.select<1, 2>());
  popPtr();
}

void Assembler::signBitConstructive(Cell result, Temps<4> tmp) {
  pushPtr();
  copyField(result, tmp.get<0>());
  moveTo(result);
  signBitDestructive(tmp.select<1, 2, 3>());
  popPtr();
}

// TODO: move other unary algorithms here
