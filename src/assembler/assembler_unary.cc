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

// TODO: move other unary algorithms here
