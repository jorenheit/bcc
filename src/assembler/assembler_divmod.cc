#include "assembler.ih"

// TODO: add a flag to not throw away the other result, but keep it in the payload.
// This will greatly reduce the code for decimal printing because the mod and div will
// be able to be done in a single go. Don't forget to clear the payload though.

Assembler::Mop const Assembler::divSpec {
  .op = BinOp::Div,
  .fold = [](int x, int y) -> int { return x / y; },
  .applyWithSlot = &Assembler::divSlotBySlot,
  .applyWithConst = &Assembler::divSlotByConst
};

Assembler::Mop const Assembler::modSpec {
  .op = BinOp::Mod,
  .fold = [](int x, int y) -> int { return x % y; },
  .applyWithSlot = &Assembler::modSlotBySlot,
  .applyWithConst = &Assembler::modSlotByConst
};

void Assembler::divSlotByConst(Slot const &lhs, int denom) {

  auto lhsWasNegative = [&] -> std::optional<Slot> {
    if (not types::isSignedInteger(lhs.type)) return {};

    // For signed integers, check if the value is negative. If so, take the
    // absolute value but remember the sign.
    moveTo(lhs, lhs.type->usesValue1() ? MacroCell::Value1 : MacroCell::Value0);
    signBitConstructive(Cell{lhs, MacroCell::Flag},
			Temps<4>::select(lhs, MacroCell::Scratch0,
					 lhs, MacroCell::Scratch1,
					 lhs, MacroCell::Payload0,
					 lhs, MacroCell::Payload1));

    Slot const tmp = getTemp(ts::raw(1));
    moveTo(lhs, MacroCell::Flag);
    loopOpen(); {
      moveTo(tmp, MacroCell::Flag);
      setToValue(1);
      negateSlot(lhs);
      moveTo(lhs, MacroCell::Flag);
      zeroCell();      
    } loopClose();
    
    return tmp;
  }();

  pushPtr();
  if (lhs.type->usesValue1()) {
    // TODO: less temps: negateFlag can share slot with this one
    Slot const tmp = getTemp(ts::raw(2));
    moveTo(lhs, MacroCell::Value0);    
    divMod16Const(std::abs(denom), Cell{lhs, MacroCell::Value1},
		  Cell{lhs, MacroCell::Payload0},
		  Cell{lhs, MacroCell::Payload1},
		  Temps<8>::select(lhs, MacroCell::Scratch0,
				   lhs, MacroCell::Scratch1,
				   tmp, MacroCell::Scratch0,
				   tmp, MacroCell::Scratch1,
				   tmp + 1, MacroCell::Scratch0,
				   tmp + 1, MacroCell::Scratch1,
				   tmp + 1, MacroCell::Payload0,
				   tmp + 1, MacroCell::Payload1));

    // TODO: leave in payload if requested by caller
    moveTo(lhs, MacroCell::Payload0);
    zeroCell();
    moveTo(lhs, MacroCell::Payload1);
    zeroCell();
    
  } else {
    Slot const tmp = getTemp(ts::raw(1));
    moveTo(lhs, MacroCell::Value0);    
    divModConst(std::abs(denom), Cell{lhs, MacroCell::Scratch0},
		Temps<5>::select(lhs, MacroCell::Scratch1,
				 lhs, MacroCell::Payload0,
				 lhs, MacroCell::Payload1,
				 tmp, MacroCell::Scratch0,
				 tmp, MacroCell::Scratch1));

    // TODO: leave in payload if requested by caller
    moveTo(lhs, MacroCell::Scratch0);
    zeroCell();
  }

  if (lhsWasNegative.has_value()) {
    moveTo(*lhsWasNegative, MacroCell::Flag);
    if (denom < 0) {
      notDestructive(Cell{*lhsWasNegative, MacroCell::Scratch0});
    }
    loopOpen(); {
      zeroCell();
      moveTo(lhs);
      negateSlot(lhs);
      moveTo(*lhsWasNegative, MacroCell::Flag);
    } loopClose();
  }

  
  popPtr();
}

void Assembler::divSlotBySlot(Slot const &lhs, Slot const &rhs) {

  Slot const rhsCopy = getTemp(rhs.type);
  assignSlot(rhsCopy, rhs);

  auto resultIsNegative = [&] -> std::optional<Slot> {
    if (!types::isSignedInteger(lhs.type) && !types::isSignedInteger(rhs.type)) return {};

    Slot const tmp = getTemp(ts::raw(1));

    moveTo(lhs, lhs.type->usesValue1() ? MacroCell::Value1 : MacroCell::Value0);
    signBitConstructive(Cell{lhs, MacroCell::Flag},
			Temps<4>::select(lhs, MacroCell::Scratch0,
					 lhs, MacroCell::Scratch1,
					 lhs, MacroCell::Payload0,
					 lhs, MacroCell::Payload1));

    moveTo(lhs, MacroCell::Flag);
    loopOpen(); {
      // lhs < 0  ==>  negate LHS and set tmp flag
      zeroCell();      
      negateSlot(lhs);
      moveTo(tmp, MacroCell::Flag);
      setToValue(1);
      moveTo(lhs, MacroCell::Flag);
    } loopClose();

    
    moveTo(rhsCopy, rhsCopy.type->usesValue1() ? MacroCell::Value1 : MacroCell::Value0);
    signBitConstructive(Cell{rhsCopy, MacroCell::Flag},
			Temps<4>::select(rhsCopy, MacroCell::Scratch0,
					 rhsCopy, MacroCell::Scratch1,
					 rhsCopy, MacroCell::Payload0,
					 rhsCopy, MacroCell::Payload1));

    moveTo(rhsCopy, MacroCell::Flag);
    loopOpen(); {
      zeroCell();      
      negateSlot(rhsCopy);
      // rhs < 0  ==> set tmp flag only if it was not already set and negate rhs
      moveTo(tmp, MacroCell::Flag);
      notDestructive(Cell{tmp, MacroCell::Scratch0});
      moveTo(rhsCopy, MacroCell::Flag);
    } loopClose();

    return tmp;
  }();


  pushPtr();
  if (lhs.type->usesValue1() || rhs.type->usesValue1()) {
    moveTo(lhs, MacroCell::Value0);    
    divMod16Destructive(Cell{lhs, MacroCell::Value1},
			Cell{rhsCopy, MacroCell::Value0},
			Cell{rhsCopy, MacroCell::Value1},
			Cell{lhs, MacroCell::Payload0},
			Cell{lhs, MacroCell::Payload1},
			Temps<8>::select(lhs, MacroCell::Scratch0,
					 lhs, MacroCell::Scratch1,
					 rhs, MacroCell::Scratch0,
					 rhs, MacroCell::Scratch1,
					 rhsCopy, MacroCell::Scratch0,
					 rhsCopy, MacroCell::Scratch1,
					 rhsCopy, MacroCell::Payload0,
					 rhsCopy, MacroCell::Payload1));
    moveTo(lhs, MacroCell::Payload0);
    zeroCell();
    moveTo(lhs, MacroCell::Payload1);
    zeroCell();
    
  } else {
    moveTo(lhs, MacroCell::Value0);    
    divModDestructive(Cell{rhsCopy, MacroCell::Value0},
		      Cell{lhs, MacroCell::Scratch0},
		      Temps<5>::select(lhs, MacroCell::Scratch1,
				       lhs, MacroCell::Payload0,
				       lhs, MacroCell::Payload1,
				       rhsCopy, MacroCell::Scratch0,
				       rhsCopy, MacroCell::Scratch1));

    moveTo(lhs, MacroCell::Scratch0);
    zeroCell();
  }

  if (resultIsNegative.has_value()) {
    moveTo(*resultIsNegative, MacroCell::Flag);
    loopOpen(); {
      zeroCell();
      negateSlot(lhs);
    } loopClose();
  }
  
  popPtr();
}

void Assembler::modSlotByConst(Slot const &lhs, int denom) {

  auto resultIsNegative = [&] -> std::optional<Slot> {
    if (not types::isSignedInteger(lhs.type)) return {};

    // For signed integers, the sign of the result is equal to the sign of the LHS
    moveTo(lhs, lhs.type->usesValue1() ? MacroCell::Value1 : MacroCell::Value0);
    signBitConstructive(Cell{lhs, MacroCell::Flag},
			Temps<4>::select(lhs, MacroCell::Scratch0,
					 lhs, MacroCell::Scratch1,
					 lhs, MacroCell::Payload0,
					 lhs, MacroCell::Payload1));

    Slot const tmp = getTemp(ts::raw(1));
    moveTo(lhs, MacroCell::Flag);
    loopOpen(); {
      moveTo(tmp, MacroCell::Flag);
      setToValue(1);
      negateSlot(lhs);
      moveTo(lhs, MacroCell::Flag);
      zeroCell();      
    } loopClose();
    
    return tmp;
  }();

  
  pushPtr();
  moveTo(lhs, MacroCell::Value0);    
  if (lhs.type->usesValue1()) {
    Slot const tmp = getTemp(ts::raw(2));
    divMod16Const(std::abs(denom), Cell{lhs, MacroCell::Value1},
		  Cell{lhs, MacroCell::Payload0},
		  Cell{lhs, MacroCell::Payload1},
		  Temps<8>::select(lhs, MacroCell::Scratch0,
				   lhs, MacroCell::Scratch1,
				   tmp, MacroCell::Scratch0,
				   tmp, MacroCell::Scratch1,
				   tmp + 1, MacroCell::Scratch0,
				   tmp + 1, MacroCell::Scratch1,
				   tmp + 1, MacroCell::Payload0,
				   tmp + 1, MacroCell::Payload1));

    moveTo(lhs, MacroCell::Payload0);
    moveField(Cell{lhs, MacroCell::Value0});
    moveTo(lhs, MacroCell::Payload1);
    moveField(Cell{lhs, MacroCell::Value1});
  } else {
    Slot const tmp = getTemp(ts::raw(1));
    divModConst(std::abs(denom), Cell{lhs, MacroCell::Payload0},
		Temps<5>::select(lhs, MacroCell::Scratch0,
				 lhs, MacroCell::Scratch1,
				 tmp, MacroCell::Scratch0,
				 tmp, MacroCell::Scratch1,
				 tmp, MacroCell::Payload0));

    moveTo(lhs, MacroCell::Payload0);
    moveField(Cell{lhs, MacroCell::Value0});
  }

  if (resultIsNegative.has_value()) {
    moveTo(*resultIsNegative, MacroCell::Flag);
    loopOpen(); {
      zeroCell();
      negateSlot(lhs);
    } loopClose();
  }

  popPtr();
}

void Assembler::modSlotBySlot(Slot const &lhs, Slot const &rhs) {

  auto resultIsNegative = [&] -> std::optional<Slot> {
    if (not types::isSignedInteger(lhs.type)) return {};

    // For signed integers, the sign of the result is equal to the sign of the LHS
    moveTo(lhs, lhs.type->usesValue1() ? MacroCell::Value1 : MacroCell::Value0);
    signBitConstructive(Cell{lhs, MacroCell::Flag},
			Temps<4>::select(lhs, MacroCell::Scratch0,
					 lhs, MacroCell::Scratch1,
					 lhs, MacroCell::Payload0,
					 lhs, MacroCell::Payload1));

    Slot const tmp = getTemp(ts::raw(1));
    moveTo(lhs, MacroCell::Flag);
    loopOpen(); {
      moveTo(tmp, MacroCell::Flag);
      setToValue(1);
      negateSlot(lhs);
      moveTo(lhs, MacroCell::Flag);
      zeroCell();      
    } loopClose();
    
    return tmp;
  }();


  Slot const rhsCopy = getTemp(rhs.type);
  assignSlot(rhsCopy, rhs);
  if (types::isSignedInteger(rhs.type)) {
    absSlot(rhsCopy);
  }
  
  pushPtr();
  if (lhs.type->usesValue1() || rhs.type->usesValue1()) {
    moveTo(lhs, MacroCell::Value0);    
    divMod16Destructive(Cell{lhs, MacroCell::Value1},
			Cell{rhsCopy, MacroCell::Value0},
			Cell{rhsCopy, MacroCell::Value1},
			Cell{lhs, MacroCell::Payload0},
			Cell{lhs, MacroCell::Payload1},
			Temps<8>::select(lhs, MacroCell::Scratch0,
					 lhs, MacroCell::Scratch1,
					 rhs, MacroCell::Scratch0,
					 rhs, MacroCell::Scratch1,
					 rhsCopy, MacroCell::Scratch0,
					 rhsCopy, MacroCell::Scratch1,
					 rhsCopy, MacroCell::Payload0,
					 rhsCopy, MacroCell::Payload1));
    moveTo(lhs, MacroCell::Payload0);
    moveField(Cell{lhs, MacroCell::Value0});
    moveTo(lhs, MacroCell::Payload1);
    moveField(Cell{lhs, MacroCell::Value1});
    
  } else {
    moveTo(lhs, MacroCell::Value0);    
    divModDestructive(Cell{rhsCopy, MacroCell::Value0},
		      Cell{lhs, MacroCell::Scratch0},
		      Temps<5>::select(lhs, MacroCell::Scratch1,
				       lhs, MacroCell::Payload0,
				       lhs, MacroCell::Payload1,
				       rhsCopy, MacroCell::Scratch0,
				       rhsCopy, MacroCell::Scratch1));

    moveTo(lhs, MacroCell::Scratch0);
    moveField(Cell{lhs, MacroCell::Value0});
  }


  if (resultIsNegative.has_value()) {
    moveTo(*resultIsNegative, MacroCell::Flag);
    loopOpen(); {
      zeroCell();
      negateSlot(lhs);
    } loopClose();
  }

  
  popPtr();
}

void Assembler::divModConst(int denom, Cell modResult, Temps<5> tmp) {
  pushPtr();
  Cell const divResult = _dp.current();

  moveTo(modResult); setToValue(0);
  if (denom == 1) {
    popPtr();
    return;
  }
  if (denom == 0) {
    moveTo(divResult); zeroCell(); subConst(1);
    popPtr();
    return;
  }

  Cell const numeratorIsZero = tmp.get<0>();
  Cell const loopFlag = tmp.get<1>();
  Cell const numCopy = tmp.get<2>();
  Cell const denomCopy = tmp.get<3>();

  moveTo(divResult);  moveField(numCopy);
  moveTo(denomCopy);  setToValue(denom & 0xff);
  moveTo(loopFlag);   setToValue(1);

  moveTo(numCopy);
  notConstructive(numeratorIsZero, tmp.select<4>());
  moveTo(numeratorIsZero);
  loopOpen(); {
    zeroCell();
    // 0 / x -> divResult and modResult remain 0 -> return
    moveTo(loopFlag);
    setToValue(0);
    moveTo(numeratorIsZero);
  } loopClose();

  moveTo(loopFlag);
  loopOpen(); {    

    moveTo(modResult); inc();
    moveTo(numCopy);   dec();
    moveTo(denomCopy); dec();

    // Repurpose flag-cell
    Cell const denominatorIsZero = numeratorIsZero;
    moveTo(denomCopy);
    notConstructive(denominatorIsZero, tmp.select<4>());
    moveTo(denominatorIsZero);
    loopOpen(); {
      zeroCell();
      
      // denomCopy was decremented all the way to 0 -> increase div-result, restore numCopy,  and reset mod-result
      moveTo(divResult); inc();
      moveTo(modResult); zeroCell();
      moveTo(denomCopy);
      setToValue(denom & 0xff);
      
      moveTo(denominatorIsZero);
    } loopClose();

    // Repurpose flag-call
    Cell const numeratorIsZero = denominatorIsZero;
    moveTo(numCopy);
    notConstructive(numeratorIsZero, tmp.select<4>());
    moveTo(numeratorIsZero);
    loopOpen(); {
      zeroCell();
      moveTo(loopFlag); setToValue(0);
      moveTo(numeratorIsZero);
    } loopClose();

    moveTo(loopFlag);
  } loopClose();

  // Cleanup
  moveTo(denomCopy); zeroCell();

  popPtr();
}

void Assembler::divMod16Const(int denom, Cell high, Cell modResultLow, Cell modResultHigh, Temps<8> tmp) {
  pushPtr();

  Cell const divResultLow = _dp.current();
  Cell const divResultHigh = high;
  
  moveTo(modResultLow);  setToValue(0);
  moveTo(modResultHigh); setToValue(0);

  if (denom == 1) {
    popPtr();
    return;
  }
  
  if (denom == 0) {
    moveTo(divResultLow);  zeroCell(); subConst(1);
    moveTo(divResultHigh); zeroCell(); subConst(1);
    popPtr();
    return;
  }

  Cell const numeratorIsZero = tmp.get<0>();
  Cell const loopFlag = tmp.get<1>();
  Cell const numCopyLow = tmp.get<2>();
  Cell const numCopyHigh = tmp.get<3>();  
  Cell const denomCopyLow = tmp.get<4>();
  Cell const denomCopyHigh = tmp.get<5>();

  moveTo(divResultLow);  moveField(numCopyLow);
  moveTo(divResultHigh); moveField(numCopyHigh);
  moveTo(denomCopyLow);  setToValue(denom & 0xff);
  moveTo(denomCopyHigh); setToValue((denom >> 8) & 0xff);
  moveTo(loopFlag);      setToValue(1);

  moveTo(numCopyLow);
  not16Constructive(numCopyHigh, numeratorIsZero, tmp.select<6, 7>());
  moveTo(numeratorIsZero);
  loopOpen(); { // if numerator is 0, return immediately (divResult and modResult remain 0)
    zeroCell();
    moveTo(loopFlag);
    setToValue(0);
    moveTo(numeratorIsZero);
  } loopClose();

  moveTo(loopFlag);
  loopOpen(); { // Division algorithm starts here

    // increase mod while decrementing numerator and denominator
    moveTo(modResultLow); inc16(modResultHigh, tmp.select<6, 7>());
    moveTo(numCopyLow);   dec16(numCopyHigh, tmp.select<6, 7>());
    moveTo(denomCopyLow); dec16(denomCopyHigh, tmp.select<6, 7>());

    // Check if the denominator (copy) has been decremented to 0
    Cell const denominatorIsZero = numeratorIsZero;
    moveTo(denomCopyLow);    
    not16Constructive(denomCopyHigh, denominatorIsZero, tmp.select<6, 7>());
    moveTo(denominatorIsZero);
    loopOpen(); {
      zeroCell();
      
      // denomCopy was decremented all the way to 0 -> increase div-result, restore numCopy, and reset mod-result
      moveTo(divResultLow);  inc16(divResultHigh, tmp.select<6, 7>());
      moveTo(modResultLow);  zeroCell();
      moveTo(modResultHigh); zeroCell();
      moveTo(denomCopyLow);  setToValue(denom & 0xff);
      moveTo(denomCopyHigh); setToValue((denom >> 8) & 0xff);
      
      moveTo(denominatorIsZero);
    } loopClose();

    // Check if the numerator (copy) has been decremented to 0 -> we're done
    Cell const numeratorIsZero = denominatorIsZero;
    moveTo(numCopyLow);
    not16Constructive(numCopyHigh, numeratorIsZero, tmp.select<6, 7>());
    moveTo(numeratorIsZero);
    loopOpen(); {
      zeroCell();
      moveTo(loopFlag); setToValue(0);
      moveTo(numeratorIsZero);
    } loopClose();

    moveTo(loopFlag);
  } loopClose();

  // Cleanup
  moveTo(denomCopyLow);  zeroCell();
  moveTo(denomCopyHigh); zeroCell();

  popPtr();
}


void Assembler::divModDestructive(Cell denom, Cell modResult, Temps<5> tmp) {
  pushPtr();

  Cell const divResult = _dp.current();
  Cell const zeroFlag = tmp.get<0>();
  Cell const loopFlag = tmp.get<1>();
  Cell const numCopy = tmp.get<2>();
  Cell const denomCopy = tmp.get<3>();

  moveTo(modResult); setToValue(0);
  moveTo(divResult); moveField(numCopy);
  moveTo(denom);     copyField(denomCopy, tmp.select<4>());
  moveTo(loopFlag);  setToValue(1);

  // Division by 0
  moveTo(denomCopy);
  notConstructive(zeroFlag, tmp.select<4>());
  moveTo(zeroFlag);
  loopOpen(); {
    moveTo(divResult); dec();
    moveTo(loopFlag);  setToValue(0);
    moveTo(zeroFlag);  zeroCell();
  } loopClose();

  // Zero in the numerator
  moveTo(numCopy);
  notConstructive(zeroFlag, tmp.select<4>());
  moveTo(zeroFlag);
  loopOpen(); {
    moveTo(divResult); zeroCell();
    moveTo(loopFlag);  setToValue(0);
    moveTo(zeroFlag);  zeroCell();
  } loopClose();

  // General case
  moveTo(loopFlag);
  loopOpen(); {    

    moveTo(modResult); inc();
    moveTo(numCopy);   dec();
    moveTo(denomCopy); dec();

    moveTo(denomCopy);
    notConstructive(zeroFlag, tmp.select<4>());
    moveTo(zeroFlag);
    loopOpen(); {
      zeroCell();
      // denomCopy was decremented all the way to 0 -> increase div-result, restore numCopy,  and reset mod-result
      moveTo(divResult); inc();
      moveTo(modResult); zeroCell();
      moveTo(denom);     copyField(denomCopy, tmp.select<4>());

      moveTo(zeroFlag);
    } loopClose();

    moveTo(numCopy);
    notConstructive(zeroFlag, tmp.select<4>());
    moveTo(zeroFlag);
    loopOpen(); {
      zeroCell();
      moveTo(loopFlag); setToValue(0);
      moveTo(zeroFlag);
    } loopClose();
    
    moveTo(loopFlag);
  } loopClose();

  // Cleanup
  moveTo(denom);     zeroCell();
  moveTo(denomCopy); zeroCell();

  popPtr();
}

void Assembler::divMod16Destructive(Cell high, Cell denomLow, Cell denomHigh, Cell modResultLow, Cell modResultHigh, Temps<8> tmp) {
  pushPtr();

  Cell const divResultLow  = _dp.current();
  Cell const divResultHigh = high;
  Cell const zeroFlag      = tmp.get<0>();
  Cell const loopFlag      = tmp.get<1>();
  Cell const numCopyLow    = tmp.get<2>();
  Cell const numCopyHigh   = tmp.get<3>();
  Cell const denomCopyLow  = tmp.get<4>();
  Cell const denomCopyHigh = tmp.get<5>();

  moveTo(modResultLow);  setToValue(0);
  moveTo(modResultHigh); setToValue(0);
  moveTo(divResultLow);  moveField(numCopyLow);
  moveTo(divResultHigh); moveField(numCopyHigh);
  moveTo(denomLow);      copyField(denomCopyLow, tmp.select<6>());
  moveTo(denomHigh);     copyField(denomCopyHigh, tmp.select<6>());
  moveTo(loopFlag);      setToValue(1);

  // Division by 0
  moveTo(denomCopyLow);
  not16Constructive(denomCopyHigh, zeroFlag, tmp.select<6, 7>());
  moveTo(zeroFlag);
  loopOpen(); {
    moveTo(divResultLow); dec16(divResultHigh, tmp.select<6, 7>());
    moveTo(loopFlag);     setToValue(0);
    moveTo(zeroFlag);     zeroCell();
  } loopClose();

  // Zero in the numerator
  moveTo(numCopyLow);
  not16Constructive(numCopyHigh, zeroFlag, tmp.select<6, 7>());
  moveTo(zeroFlag);
  loopOpen(); {
    moveTo(divResultLow);  zeroCell(); // TODO: should this be [+]? The only reason this isnt 0 is when div by zero was hit
    moveTo(divResultHigh); zeroCell();
    moveTo(loopFlag);      setToValue(0);
    moveTo(zeroFlag);      zeroCell();
  } loopClose();

  // General case
  moveTo(loopFlag);
  loopOpen(); {    

    moveTo(modResultLow); inc16(modResultHigh, tmp.select<6, 7>());
    moveTo(numCopyLow);   dec16(numCopyHigh, tmp.select<6, 7>());
    moveTo(denomCopyLow); dec16(denomCopyHigh, tmp.select<6, 7>());

    moveTo(denomCopyLow);
    not16Constructive(denomCopyHigh, zeroFlag, tmp.select<6, 7>());
    moveTo(zeroFlag);
    loopOpen(); {
      // denomCopy was decremented all the way to 0 -> increase div-result, restore numCopy,  and reset mod-result
      moveTo(divResultLow);  inc16(divResultHigh, tmp.select<6, 7>());
      moveTo(modResultLow);  zeroCell();
      moveTo(modResultHigh); zeroCell();
      moveTo(denomLow);      copyField(denomCopyLow, tmp.select<6>());
      moveTo(denomHigh);     copyField(denomCopyHigh, tmp.select<6>());
      moveTo(zeroFlag);      zeroCell();
    } loopClose();

    moveTo(numCopyLow);
    not16Constructive(numCopyHigh, zeroFlag, tmp.select<6, 7>());
    moveTo(zeroFlag);
    loopOpen(); {
      moveTo(loopFlag); setToValue(0);
      moveTo(zeroFlag); zeroCell();
    } loopClose();
    
    moveTo(loopFlag);
  } loopClose();

  // Cleanup
  moveTo(denomLow);      zeroCell();
  moveTo(denomHigh);     zeroCell();
  moveTo(denomCopyLow);  zeroCell();
  moveTo(denomCopyHigh); zeroCell();

  popPtr();
}


void Assembler::divModConstructive(Cell result, Cell denom, Cell modResult, Temps<6> tmp) {
  Cell const denomCopy = tmp.get<0>();

  pushPtr();
  copyField(result, tmp.get<1>());
  moveTo(denom);
  copyField(denomCopy, tmp.get<1>());
  moveTo(result);
  divModDestructive(denomCopy, modResult, tmp.select<1, 2, 3, 4, 5>());
  popPtr();

}


void Assembler::divMod16Constructive(Cell high,
				    Cell resultLow, Cell resultHigh,
				    Cell denomLow, Cell denomHigh,
				    Cell modResultLow, Cell modResultHigh,
				    Temps<12> tmp) {

  Cell const low            = _dp.current();
  Cell const denomLowCopy   = tmp.get<1>();
  Cell const denomHighCopy  = tmp.get<2>();
  
  pushPtr();
  moveTo(low);       copyField(resultLow, tmp.select<3>());
  moveTo(high);      copyField(resultHigh, tmp.select<3>());
  moveTo(denomLow);  copyField(denomLowCopy, tmp.select<3>());
  moveTo(denomHigh); copyField(denomHighCopy, tmp.select<3>());

  moveTo(resultLow);
  divMod16Destructive(resultHigh,
		      denomLowCopy, denomHighCopy,
		      modResultLow, modResultHigh,
		      tmp.select<4, 5, 6, 7, 8, 9, 10, 11>());
  popPtr();
}

