#include "assembler.ih"

// TODO: factor out into a writeSlot? That's more consistent with the other API functions
void Assembler::writeOutImpl(Expression const &rhs, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_FUNCTION_BLOCK();

  pushPtr();

  Slot const slot = rhs.hasSlot()
    ? rhs.slot()->materialize(*this)
    : getTemp(rhs.literal());

  // Special case for Strings: look for NULL terminator
  if (types::isString(slot.type)) {
    moveTo(slot, MacroCell::Value0);
    setSeekMarker();

    emit<primitive::CopyData>(MacroCell::Value0, MacroCell::Flag, MacroCell::Scratch0);
    switchField(MacroCell::Flag);
    loopOpen(); {
      zeroCell();
      switchField(MacroCell::Value0);
      emit<primitive::Out>();
      emit<primitive::MovePointerRelative>(MacroCell::FieldCount);

      // Check if end of string was reached by using the current value as a flag.
      // If NULL terminator hit, we exit the loop and go back to start.
      emit<primitive::CopyData>(MacroCell::Value0, MacroCell::Flag, MacroCell::Scratch0);
      switchField(MacroCell::Flag);
    } loopClose();

    // We hit the end of the string -> return to seek marker (no payload, check current as well)
    seek(MacroCell::SeekMarker, primitive::Left, {}, true);
    resetSeekMarker();
    popPtr();
    return;
  }

  // All other types: just output all Values sequentially
  for (int i = 0; i != slot.type->size(); ++i) {
    moveTo(slot + i, MacroCell::Value0);
    emit<primitive::Out>();
    if (slot.type->usesValue1()) {
      moveTo(slot + i, MacroCell::Value1);
      emit<primitive::Out>();
    }
  }
  popPtr();
}

void Assembler::printImpl(Expression const &val, API_CTX) {
  
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_FUNCTION_BLOCK();
  
  if (types::isInteger(val.type())) {
    // TODO: option to force inlining of the builtin calls?
    
    BuiltinFunction const func = val.type()->usesValue1()
      ? (types::isUnsignedInteger(val.type()) ? BuiltinFunction::PrintUnsigned16 : BuiltinFunction::PrintSigned16)
      : (types::isUnsignedInteger(val.type()) ? BuiltinFunction::PrintUnsigned8 : BuiltinFunction::PrintSigned8);
    
    _usedBuiltinFunctions.insert(func);

    callFunctionImpl(builtinFunctionName(func), {}, { val }, API_FWD);
    return;
  }
  API_REQUIRE(false, error::ErrorCode::NotPrintable, "print is not supported for values of type '", val.type()->str(), "'.");
}


void Assembler::printUnsignedImpl(Expression const &val) {
  assert(types::isUnsignedInteger(val.type()));

  if (val.isLiteral()) {
    int x = literal::cast<types::IntegerType>(val.literal())->semanticValue();
    std::vector<int> digits;
    while (x > 0) {
      digits.push_back(x % 10);
      x /= 10;
    }

    Slot const tmp = getTemp(ts::i8());
    for (int i = digits.size() - 1; i >= 0; --i) {
      moveTo(tmp);
      setToValue(digits[i] + '0');
      emit<primitive::Out>();
    }
  }
  else {
    Slot const valSlot = [&] {
      Slot const materialized = val.slot()->materialize(*this);
      if (not val.slot()->direct()) {
	return materialized; // already a disposable temp
      }
      Slot const copy = getTemp(val.type());
      assignSlot(copy, materialized);
      return copy;
    }();

    printIntegerSlotDestructive(valSlot);
  }

}


void Assembler::printSignedImpl(Expression const &val) {
  assert(types::isSignedInteger(val.type()));
  
  if (val.isLiteral()) {
    int x = literal::cast<types::IntegerType>(val.literal())->semanticValue();
    bool const negative = x < 0;

    std::vector<int> digits;
    x = std::abs(x);
    while (x > 0) {
      digits.push_back(x % 10);
      x /= 10;
    }

    // TMP to construct character values into
    Slot const tmp = getTemp(ts::i8());
    
    if (negative) {
      moveTo(tmp);
      setToValue('-');
      emit<primitive::Out>();
    }
    
    for (int i = digits.size() - 1; i >= 0; --i) {
      moveTo(tmp);
      setToValue(digits[i] + '0');
      emit<primitive::Out>();
    }
  }
  else {
    Slot valSlot = [&] {
      Slot const materialized = val.slot()->materialize(*this);
      if (not val.slot()->direct()) {
	return materialized; // already a disposable temp
      }
      Slot const copy = getTemp(val.type());
      assignSlot(copy, materialized);
      return copy;
    }();

    // Construct sign bit in the flag and use that to determine whether to print a - sign.
    moveTo(valSlot, valSlot.type->usesValue1() ? MacroCell::Value1 : MacroCell::Value0);
    signBitConstructive(Cell{valSlot, MacroCell::Flag},
			Temps<4>::select(valSlot, MacroCell::Scratch0,
					 valSlot, MacroCell::Scratch1,
					 valSlot, MacroCell::Payload0,
					 valSlot, MacroCell::Payload1));

    moveTo(valSlot, MacroCell::Flag);
    loopOpen(); {
      // Negate valSlot while we're here
      negateSlot(valSlot);
      setToValue('-');
      emit<primitive::Out>();
      zeroCell();
    } loopClose();
    
    valSlot.type = valSlot.type->usesValue1() ? ts::i16() : ts::i8();
    printIntegerSlotDestructive(valSlot);    
  }
}

void Assembler::printIntegerSlotDestructive(Slot const &valSlot) {
  assert(types::isInteger(valSlot.type));

  pushPtr();
  
  // We can destroy the contents of valSlot while working on it.
  int const maxDigits = valSlot.type->usesValue1() ? 5 : 3;
	  
  Slot const digits = getTemp(ts::raw(maxDigits));
  for (int i = 0; i != 5; ++i) {
    Slot const currentDigitSlot = digits.sub(ts::i16(), i);
    assignSlot(currentDigitSlot, valSlot);

    // TODO: these can be combined; the divmod algorithm already computes both.
    modSlotByConst(currentDigitSlot, 10);
    divSlotByConst(valSlot, 10);
  }
	
  // Already add '0' to the 1's digit (at the base) to make sure there is at least 1 nonzero.
  // Plant a marker at the base of the array.
  moveTo(digits);
  addConst('0');	  
  setSeekMarker();
	
  // Start at right-most digit and move left until first nonzero is found.
  moveTo(digits + maxDigits - 1);
  seek(MacroCell::Value0, primitive::Left, {}, true);

  // Now just print every character until the start of the string has been reached (seekMarker).
  // Don't print the 1's digit in the loop.
  switchField(MacroCell::SeekMarker);
  notConstructive(Cell{_dp.current().offset, MacroCell::Flag},
		  Temps<1>::select(_dp.current().offset, MacroCell::Scratch0));
  switchField(MacroCell::Flag);
  loopOpen(); {
    zeroCell();

    switchField(MacroCell::Value0);
    addConst('0');
    emit<primitive::Out>();
	    
    emit<primitive::MovePointerRelative>(-1 * MacroCell::FieldCount);
    switchField(MacroCell::SeekMarker);
    notConstructive(Cell{_dp.current().offset, MacroCell::Flag},
		    Temps<1>::select(_dp.current().offset, MacroCell::Scratch0));
    switchField(MacroCell::Flag);
  } loopClose();

  // At seekmarker -> print 1's digit and clear seekMarker
  switchField(MacroCell::Value0);
  emit<primitive::Out>();
  resetSeekMarker();

  // Rebase datapointer to the start of the digits-array
  _dp.set(Cell{digits, MacroCell::Value0});


  popPtr();
}
