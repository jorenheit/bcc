#include "compiler.ih"

// TODO: group binops in files: compiler_mul.cc, etc

void Compiler::loopOpen(std::string const &tag) {
  emit<primitive::LoopOpen>(tag);
}

void Compiler::loopClose(std::string const &tag) {
  emit<primitive::LoopClose>(tag);
}

void Compiler::switchField(MacroCell::Field field) {
  assert(_currentSeq != nullptr);
  emit<primitive::MovePointerRelative>(field - _dp.current().field);
  _dp.set(field);
}

void Compiler::moveTo(int offset, MacroCell::Field field) {
  assert(_currentSeq != nullptr);

  switchField(field);
  moveRel(offset - _dp.current().offset);
}

void Compiler::moveTo(Cell dest) {
  moveTo(dest.offset, dest.field);
}

void Compiler::moveRel(int diff) {
  emit<primitive::MovePointerRelative>(diff * MacroCell::FieldCount);
  _dp.moveRelative(diff);
}

void Compiler::moveToOrigin() {
  moveTo(0);
}


void Compiler::zeroCell() { 
  emit<primitive::ZeroCell>();
}

void Compiler::setToValue(int value) { 
  zeroCell();
  addConst(value & 0xff);
}

void Compiler::setToValue16(int value, Cell high) { 
  pushPtr();
  setToValue(value & 0xff);
  moveTo(high);
  setToValue((value >> 8) & 0xff);
  popPtr();
}

void Compiler::addConst(int delta) {
  emit<primitive::ChangeBy>(delta);
}

void Compiler::addConstAndCarry(int delta, Cell carry, Temps<3> tmp) {
  pushPtr();

  if (delta == 0) {
    moveTo(carry);
    zeroCell();
    popPtr();
    return;
  }

  Cell const resultCopy = tmp.get<0>();      
  copyField(carry, tmp.select<1>());
  addConst(delta);
  copyField(resultCopy, tmp.select<1>());

  moveTo(carry);
  if (delta > 0) {
    greaterDestructive(resultCopy, tmp.select<1, 2>());
  } else {
    lessDestructive(resultCopy, tmp.select<1, 2>());
  }

  popPtr();
}  

void Compiler::subConst(int delta) {
  addConst(-delta);
}

void Compiler::mulConst(int factor, Temps<3> tmp) {
  // TODO: optimize for powers of 2
  // TODO: big factors should have runtime implementation

  if (factor == 0) {
    zeroCell();
    return;
  }
  if (factor == 1) return;

  pushPtr();
  Cell const current = _dp.current();
  Cell const copy1 = tmp.get<0>();
  Cell const copy2 = tmp.get<1>();
  
  copyField(copy1, tmp.select<2>());
  copyField(copy2, tmp.select<2>());
  
  for (int i = 0; i != factor - 1; ++i) {
    moveTo(current);    
    addDestructive(copy1);
    moveTo(copy2);
    copyField(copy1, tmp.select<2>());
  }
  popPtr();
}

void Compiler::mul16Const(int factor, Cell high, Temps<8> tmp) {
  if (factor == 0) {
    pushPtr();
    zeroCell();
    moveTo(high);
    zeroCell();
    popPtr();
    return;
  }

  if (factor == 1) return;

  pushPtr();
  Cell const current     = _dp.current();
  Cell const copy1low  = tmp.get<0>();
  Cell const copy1high = tmp.get<1>();
  Cell const copy2low  = tmp.get<2>();
  Cell const copy2high = tmp.get<3>();

  moveTo(current);
  copyField(copy1low,  tmp.select<4>());
  copyField(copy2low,  tmp.select<4>());

  moveTo(high);
  copyField(copy1high, tmp.select<4>());
  copyField(copy2high, tmp.select<4>());

  for (int i = 0; i != factor - 1; ++i) {
    moveTo(current);
    add16Destructive(high, copy1low, copy1high, tmp.select<4, 5, 6, 7>());
    moveTo(copy2low);
    copyField(copy1low, tmp.select<4>());
    moveTo(copy2high);
    copyField(copy1high, tmp.select<4>());      
  }
  
  popPtr();
}


void Compiler::mulDestructive(Cell factor, Temps<3> tmp) {
  pushPtr();
  
  Cell const current = _dp.current();
  Cell const copy1 = tmp.get<0>();
  Cell const copy2 = tmp.get<1>();
  
  copyField(copy1, tmp.select<2>());
  copyField(copy2, tmp.select<2>());
  zeroCell();
  
  moveTo(factor);
  loopOpen(); {
    dec();
    moveTo(current);
    addDestructive(copy1);
    moveTo(copy2);
    copyField(copy1, tmp.select<2>());
    moveTo(factor);
  } loopClose();

  moveTo(copy1); zeroCell();
  moveTo(copy2); zeroCell();
  
  popPtr();
}

// TODO: make a 16-bit version that uses the 8-bit version of mul rather than brute force repeat the addition.
void Compiler::mul16Destructive(Cell high, Cell factorLow, Cell factorHigh, Temps<9> tmp) {

  pushPtr();
  
  Cell const current   = _dp.current();
  Cell const copy1low  = tmp.get<0>();
  Cell const copy2low  = tmp.get<1>();
  Cell const copy1high = tmp.get<2>();
  Cell const copy2high = tmp.get<3>();
  Cell const factorNonzero = tmp.get<4>();

  moveTo(current);
  copyField(copy1low, tmp.select<5>());
  copyField(copy2low, tmp.select<5>());
  zeroCell();

  moveTo(high);
  copyField(copy1high, tmp.select<5>());
  copyField(copy2high, tmp.select<5>());
  zeroCell();

  auto computeFactorNonzero = [&]() {
    moveTo(factorNonzero);
    zeroCell(); // TODO: this is superfluous right?
    moveTo(factorLow);
    orConstructive(factorNonzero, factorHigh, tmp.select<5, 6>());
    moveTo(factorNonzero);
  };
  
  computeFactorNonzero();  
  loopOpen(); {
    moveTo(factorLow);
    dec16(factorHigh, tmp.select<5, 6>());

    moveTo(current);
    add16Destructive(high, copy1low, copy1high, tmp.select<5, 6, 7, 8>());

    moveTo(copy2low);
    copyField(copy1low, tmp.select<5>());
    moveTo(copy2high);
    copyField(copy1high, tmp.select<5>());

    computeFactorNonzero();
  } loopClose();


  moveTo(copy1low);  zeroCell();
  moveTo(copy1high); zeroCell();
  moveTo(copy2low);  zeroCell();
  moveTo(copy2high); zeroCell();
  
  popPtr();
}

void Compiler::mulConstructive(Cell result, Cell factor, Temps<4> tmp) {
  Cell const factorCopy = tmp.get<0>();

  pushPtr();
  copyField(result, tmp.get<1>());
  moveTo(factor);
  copyField(factorCopy, tmp.get<1>());
  moveTo(result);
  mulDestructive(factorCopy, tmp.select<1, 2, 3>());
  popPtr();
}


void Compiler::mul16Constructive(Cell high, Cell resultLow, Cell resultHigh, Cell factorLow, Cell factorHigh, Temps<11> tmp) {

  Cell const & low      = _dp.current();
  Cell const & factorLowCopy  = tmp.get<0>();
  Cell const & factorHighCopy = tmp.get<1>();
  
  pushPtr();
  moveTo(low);       copyField(resultLow, tmp.select<2>());
  moveTo(high);      copyField(resultHigh, tmp.select<2>());
  moveTo(factorLow);  copyField(factorLowCopy, tmp.select<2>());
  moveTo(factorHigh); copyField(factorHighCopy, tmp.select<2>());

  moveTo(resultLow);
  mul16Destructive(resultHigh, factorLowCopy, factorHighCopy, tmp.select<2, 3, 4, 5, 6, 7, 8, 9, 10>());
  popPtr();
}

void Compiler::divModConst(int denom, Cell modResult, Temps<5> tmp) {
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

void Compiler::divMod16Const(int denom, Cell high, Cell modResultLow, Cell modResultHigh, Temps<8> tmp) {
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


void Compiler::divModDestructive(Cell denom, Cell modResult, Temps<5> tmp) {
  pushPtr();

  Cell const divResult = _dp.current();
  Cell const zeroFlag = tmp.get<0>();
  Cell const loopFlag = tmp.get<1>();
  Cell const numCopy = tmp.get<2>();
  Cell const denomCopy = tmp.get<3>();

  moveTo(modResult); setToValue(0);
  moveTo(divResult); moveField(numCopy);
  moveTo(denom);     copyField(denomCopy);
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

void Compiler::divMod16Destructive(Cell high, Cell denomLow, Cell denomHigh, Cell modResultLow, Cell modResultHigh, Temps<8> tmp) {
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


void Compiler::divModConstructive(Cell result, Cell denom, Cell modResult, Temps<6> tmp) {
  Cell const denomCopy = tmp.get<0>();

  pushPtr();
  copyField(result, tmp.get<1>());
  moveTo(denom);
  copyField(denomCopy, tmp.get<1>());
  moveTo(result);
  divModDestructive(denomCopy, modResult, tmp.select<1, 2, 3, 4, 5>());
  popPtr();

}


void Compiler::divMod16Constructive(Cell high,
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

void Compiler::subConstAndCarry(int delta, Cell carry, Temps<3> tmp) {
  addConstAndCarry(-delta, carry, tmp);
}

void Compiler::inc() {
  addConst(1);
}

void Compiler::dec() {
  subConst(1);
}

void Compiler::inc16(Cell high, Temps<2> tmp) {
  pushPtr();
  Cell const carry = tmp.get<0>();
  addConst(1);
  notConstructive(carry, tmp.select<1>());
  moveTo(high);
  addDestructive(carry);
  popPtr();
}

void Compiler::dec16(Cell high, Temps<2> tmp) {
  pushPtr();
  Cell const borrow = tmp.get<0>();
  copyField(borrow, tmp.select<1>());
  subConst(1);
  moveTo(borrow);
  notDestructive(tmp.select<1>());
  moveTo(high);
  subDestructive(borrow);
  popPtr();
}


void Compiler::add16Const(int delta, Cell high, Temps<4> tmp) {
  if (delta == 0) return;
  if (delta < 0) {
    sub16Const(-delta, high, tmp);
    return;
  }
  
  int const lowDelta  = delta & 0xff;
  int const highDelta = (delta >> 8) & 0xff;
  Cell const carry = tmp.get<0>();

  pushPtr();
  if (lowDelta != 0)  addConstAndCarry(lowDelta, carry, tmp.select<1, 2, 3>());
  moveTo(high);
  if (highDelta != 0) addConst(highDelta);
  addDestructive(carry);
  popPtr();
}


void Compiler::sub16Const(int delta, Cell high, Temps<4> tmp) {
  if (delta == 0) return;
  if (delta < 0) {
    add16Const(-delta, high, tmp);
    return;
  }
  
  int const lowDelta  = delta & 0xff;
  int const highDelta = (delta >> 8) & 0xff;
  Cell const carry = tmp.get<0>();

  pushPtr();
  if (lowDelta != 0)  subConstAndCarry(lowDelta, carry, tmp.select<1, 2, 3>());
  moveTo(high);
  if (highDelta != 0) subConst(highDelta);
  subDestructive(carry);
  popPtr();
}

void Compiler::addDestructive(Cell other) {
  auto [cur, oth] = getFieldIndices(_dp.current(), other);
  emit<primitive::Add>(cur, oth);
}

void Compiler::addConstructive(Cell result, Cell other, Temps<2> tmp) {
  pushPtr();
  copyField(result, tmp.get<0>());
  moveTo(other);
  copyField(tmp.get<0>(), tmp.get<1>());
  moveTo(result);
  addDestructive(tmp.get<0>());
  popPtr();
}

void Compiler::subDestructive(Cell other) {
  auto [cur, oth] = getFieldIndices(_dp.current(), other);
  emit<primitive::Subtract>(cur, oth);
}

void Compiler::subConstructive(Cell result, Cell other, Temps<2> tmp) {
  pushPtr();
  copyField(result, tmp.get<0>());
  moveTo(other);
  copyField(tmp.get<0>(), tmp.get<1>());
  moveTo(result);
  subDestructive(tmp.get<0>());
  popPtr();
}

void Compiler::addAndCarryDestructive(Cell carry, Cell other, Temps<3> tmp) {
  Cell resultCopy = tmp.get<0>();

  pushPtr();
  copyField(carry, tmp.get<1>());  
  addDestructive(other);
  copyField(resultCopy, tmp.get<1>());
  moveTo(carry); // contains old value
  greaterDestructive(resultCopy, tmp.select<1, 2>());
  popPtr();
}

void Compiler::addAndCarryConstructive(Cell result, Cell carry, Cell other, Temps<4> tmp) {
  pushPtr();
  copyField(result, tmp.get<0>());
  moveTo(other);
  copyField(tmp.get<0>(), tmp.get<1>());
  moveTo(result);
  addAndCarryDestructive(carry, tmp.get<0>(), tmp.select<1, 2, 3>()); 
  popPtr();
}

void Compiler::subAndCarryDestructive(Cell carry, Cell other, Temps<3> tmp) {
  Cell resultCopy = tmp.get<0>();

  pushPtr();
  copyField(carry, tmp.get<1>());  
  subDestructive(other);
  copyField(resultCopy, tmp.get<1>());
  moveTo(carry); // contains old value
  lessDestructive(resultCopy, tmp.select<1, 2>());
  popPtr();
}

void Compiler::subAndCarryConstructive(Cell result, Cell carry, Cell other, Temps<4> tmp) {
  pushPtr();
  copyField(result, tmp.get<0>());
  moveTo(other);
  copyField(tmp.get<0>(), tmp.get<1>());
  moveTo(result);
  subAndCarryDestructive(carry, tmp.get<0>(), tmp.select<1, 2, 3>()); 
  popPtr();
}


void Compiler::add16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<4> tmp) {

  pushPtr();
  Cell const &low   = _dp.current();
  Cell const &carry = tmp.get<0>();
  
  // add low bytes and get the carry
  moveTo(low);
  addAndCarryDestructive(carry, otherLow, tmp.select<1, 2, 3>());

  // if carry -> increment high byte
  moveTo(carry);
  loopOpen(); {
    zeroCell(); // reset carry
    moveTo(high);
    addConst(1);
    moveTo(carry);
  } loopClose();
  
  // add high bytes, ignore carry
  moveTo(high);  
  addDestructive(otherHigh);

  popPtr();
}


void Compiler::add16Constructive(Cell high, Cell resultLow, Cell resultHigh, Cell otherLow, Cell otherHigh, Temps<6> tmp) {

  Cell const & low      = _dp.current();
  Cell const & otherLowCopy  = tmp.get<0>();
  Cell const & otherHighCopy = tmp.get<1>();
  
  pushPtr();
  moveTo(low);  copyField(resultLow, tmp.select<2>());
  moveTo(high); copyField(resultHigh, tmp.select<2>());
  moveTo(otherLow);  copyField(otherLowCopy, tmp.select<2>());
  moveTo(otherHigh); copyField(otherHighCopy, tmp.select<2>());

  moveTo(resultLow);
  add16Destructive(resultHigh, otherLowCopy, otherHighCopy, tmp.select<2, 3, 4, 5>());
  popPtr();
}


void Compiler::sub16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<4> tmp) {

  Cell const &low = _dp.current();
  Cell const &carry = tmp.get<0>();

  pushPtr();

  // subtract low bytes and get the carry
  moveTo(low);
  subAndCarryDestructive(carry, otherLow, tmp.select<1, 2, 3>());

  // if carry -> increment high byte
  moveTo(carry);
  loopOpen(); {
    zeroCell(); // reset carry
    moveTo(high);
    subConst(1);
    moveTo(carry);
  } loopClose();
  
  // subtract high bytes, ignore carry
  moveTo(high);  
  subDestructive(otherHigh);

  popPtr();
}

void Compiler::sub16Constructive(Cell high, Cell resultLow, Cell resultHigh, Cell otherLow, Cell otherHigh, Temps<6> tmp) {

  Cell const &low = _dp.current();
  Cell const &otherLowCopy  = tmp.get<0>();
  Cell const &otherHighCopy = tmp.get<1>();
  
  pushPtr();
  moveTo(low);  copyField(resultLow, tmp.select<2>());
  moveTo(high); copyField(resultHigh, tmp.select<2>());
  moveTo(otherLow);  copyField(otherLowCopy, tmp.select<2>());
  moveTo(otherHigh); copyField(otherHighCopy, tmp.select<2>());

  moveTo(resultLow);
  sub16Destructive(resultHigh, otherLowCopy, otherHighCopy, tmp.select<2, 3, 4, 5>());
  popPtr();
}


void Compiler::moveField(Cell dest) {
  auto [src, dst] = getFieldIndices(_dp.current(), dest);
  if (src == dst) return;
  emit<primitive::MoveData>(src, dst);
}

void Compiler::copyField(Cell dest, Temps<1> tmp) {
  auto [src, dst, tmp0] = getFieldIndices(_dp.current(), dest, tmp.get<0>());
  emit<primitive::CopyData>(src, dst, tmp0);
}


void Compiler::notDestructive(Temps<1> tmp) {
  auto [cur, tmp0] = getFieldIndices(_dp.current(), tmp.get<0>());
  emit<primitive::Not>(cur, tmp0);
}

void Compiler::not16Destructive(Cell high, Temps<1> tmp) {
  orDestructive(high, tmp.select<0>());
  notDestructive(tmp.select<0>());
}

void Compiler::notConstructive(Cell result, Temps<1> tmp) {
  pushPtr();
  copyField(result, tmp);
  moveTo(result);
  notDestructive(tmp);
  popPtr();
}

void Compiler::not16Constructive(Cell high, Cell result, Temps<2> tmp) {
  Cell const resultHigh = tmp.get<0>();
  
  pushPtr();
  copyField(result, tmp.select<1>());
  moveTo(high);
  copyField(resultHigh, tmp.select<1>());
  moveTo(result);
  not16Destructive(resultHigh, tmp.select<1>());
  popPtr();  
}

void Compiler::orDestructive(Cell other, Temps<1> tmp) {
  auto [cur, oth, tmp0] = getFieldIndices(_dp.current(), other, tmp.get<0>());
  emit<primitive::Or>(cur, oth, tmp0);
}

void Compiler::orConstructive(Cell result, Cell other, Temps<2> tmp) {
  Cell const &otherCopy = tmp.get<0>();
  
  pushPtr();
  copyField(result, tmp.select<1>());  
  moveTo(other);
  copyField(otherCopy, tmp.select<1>());
  moveTo(result);
  orDestructive(otherCopy, tmp.select<1>());
  popPtr();
}


void Compiler::andDestructive(Cell other, Temps<1> tmp) {
  auto [cur, oth, tmp0] = getFieldIndices(_dp.current(), other, tmp.get<0>());
  emit<primitive::And>(cur, oth, tmp0);
}

void Compiler::andConstructive(Cell result, Cell other, Temps<2> tmp) { 
  Cell const &otherCopy = tmp.get<0>();
  pushPtr();
  copyField(result, tmp.select<1>());  
  moveTo(other);
  copyField(otherCopy, tmp.select<1>());
  moveTo(result);
  andDestructive(otherCopy, tmp.select<1>());
  popPtr();
}

void Compiler::compareToConstDestructive(int value, Temps<1> tmp) {
  auto [cur, tmp0] = getFieldIndices(_dp.current(), tmp.get<0>());
  emit<primitive::Cmp>(value, cur, tmp0);
}

void Compiler::compareToConstConstructive(int value, Cell result, Temps<1> tmp) {
  pushPtr();
  copyField(result, tmp);
  moveTo(result);
  compareToConstDestructive(value, tmp);
  popPtr();
}
    
void Compiler::compare16ToConstDestructive(int value, Cell high, Temps<1> tmp) {
  pushPtr();
  compareToConstDestructive(value & 0xff, tmp);
  moveTo(high);
  compareToConstDestructive((value >> 8) & 0xff, tmp);
  popPtr();

  andDestructive(high, tmp);
}

void Compiler::compare16ToConstConstructive(int value, Cell high, Cell result, Temps<2> tmp) {
  pushPtr();
  copyField(result, tmp.get<1>());
  moveTo(high);
  copyField(tmp.get<0>(), tmp.get<1>());
  moveTo(result);
  compare16ToConstDestructive(value, tmp.get<0>(), tmp.select<1>());
  popPtr();
}

void Compiler::lessDestructive(Cell other, Temps<2> tmp) { 
  auto [cur, oth, tmp0, tmp1] = getFieldIndices(_dp.current(), other, tmp.get<0>(), tmp.get<1>());
  emit<primitive::Less>(cur, oth, tmp0, tmp1);
}

void Compiler::lessConstructive(Cell result, Cell other, Temps<3> tmp) {
  Cell const &otherCopy = tmp.get<0>();
  pushPtr();
  copyField(result, tmp.select<1>());
  moveTo(other);
  copyField(otherCopy, tmp.select<1>());
  moveTo(result);
  lessDestructive(otherCopy, tmp.select<1, 2>());
  popPtr();
}

void Compiler::lessOrEqualDestructive(Cell other, Temps<2> tmp) {
  auto [cur, oth, tmp0, tmp1] = getFieldIndices(_dp.current(), other, tmp.get<0>(), tmp.get<1>());
  emit<primitive::LessOrEqual>(cur, oth, tmp0, tmp1);
}

void Compiler::lessOrEqualConstructive(Cell result, Cell other, Temps<3> tmp) {
  Cell const &otherCopy = tmp.get<0>();
  pushPtr();
  copyField(result, tmp.select<1>());
  moveTo(other);
  copyField(otherCopy, tmp.select<1>());
  moveTo(result);
  lessOrEqualDestructive(otherCopy, tmp.select<1, 2>());
  popPtr();
}


void Compiler::greaterDestructive(Cell other, Temps<2> tmp) {
  auto [cur, oth, tmp0, tmp1] = getFieldIndices(_dp.current(), other, tmp.get<0>(), tmp.get<1>());
  emit<primitive::Greater>(cur, oth, tmp0, tmp1);
}

void Compiler::greaterConstructive(Cell result, Cell other, Temps<3> tmp) {
  Cell const &otherCopy = tmp.get<0>();
  pushPtr();
  copyField(result, tmp.select<1>());
  moveTo(other);
  copyField(otherCopy, tmp.select<1>());
  moveTo(result);
  greaterDestructive(otherCopy, tmp.select<1, 2>());
  popPtr();
}

void Compiler::greaterOrEqualDestructive(Cell other, Temps<2> tmp) {
  auto [cur, oth, tmp0, tmp1] = getFieldIndices(_dp.current(), other, tmp.get<0>(), tmp.get<1>());
  emit<primitive::GreaterOrEqual>(cur, oth, tmp0, tmp1);
}

void Compiler::greaterOrEqualConstructive(Cell result, Cell other, Temps<3> tmp) {
  Cell const &otherCopy = tmp.get<0>();
  pushPtr();
  copyField(result, tmp.select<1>());
  moveTo(other);
  copyField(otherCopy, tmp.select<1>());
  moveTo(result);
  greaterOrEqualDestructive(otherCopy, tmp.select<1, 2>());
  popPtr();
}


void Compiler::equalDestructive(Cell other, Temps<2> tmp) {
  auto [cur, oth, tmp0, tmp1] = getFieldIndices(_dp.current(), other, tmp.get<0>(), tmp.get<1>());
  emit<primitive::Equal>(cur, oth, tmp0, tmp1);
}

void Compiler::equalConstructive(Cell result, Cell other, Temps<3> tmp) {
  Cell const &otherCopy = tmp.get<0>();
  pushPtr();
  copyField(result, tmp.select<1>());
  moveTo(other);
  copyField(otherCopy, tmp.select<1>());
  moveTo(result);
  equalDestructive(otherCopy, tmp.select<1, 2>());
  popPtr();
}

void Compiler::goToDynamicOffset(Cell offsetLow, Cell offsetHigh) {
  // WARNING: this leaves pointer in unknown position.
  // Make sure to leave a marker in order to be able to seek back
  
  // Copy offset (16-bit) into payload cells of the current cell  
  int const base = _dp.current().offset;
  moveTo(offsetLow);
  copyField(Cell{base, MacroCell::Payload0}, Temps<1>::select(base, MacroCell::Scratch0));
  moveTo(offsetHigh);
  copyField(Cell{base, MacroCell::Payload1}, Temps<1>::select(base, MacroCell::Scratch0));
  
  // Starting at the current offset, move right while decrementing offset until
  // both bytes have become zero. Then move the value back to the seek-marker
  moveTo(base, MacroCell::Payload0);
  orConstructive(Cell{base, MacroCell::Flag},
		 Cell{base, MacroCell::Payload1},
		 Temps<2>::select(base, MacroCell::Scratch0,
				base, MacroCell::Scratch1));
  moveTo(base, MacroCell::Flag);
  loopOpen(); {
    zeroCell();

    // Decrement the offset
    switchField(MacroCell::Payload0);
    dec16(Cell{base, MacroCell::Payload1},
	  Temps<2>::select(base, MacroCell::Scratch0,
			 base, MacroCell::Scratch1));
    
    // move payload forward by 1
    moveField(Cell{base + 1, MacroCell::Payload0});
    switchField(MacroCell::Payload1);
    moveField(Cell{base + 1, MacroCell::Payload1});

    // Follow along with pointer (raw -> compile-time offset remains at base)
    switchField(MacroCell::Payload0);    
    emit<primitive::MovePointerRelative>(MacroCell::FieldCount);

    // Flag <- (payload == 0)
    orConstructive(Cell{base, MacroCell::Flag},
		   Cell{base, MacroCell::Payload1},
		   Temps<2>::select(base, MacroCell::Scratch0,
				  base, MacroCell::Scratch1));
    switchField(MacroCell::Flag);
  } loopClose();
  moveTo(base, MacroCell::Value0);
}

void Compiler::fetchFromDynamicOffset(Cell offsetLow, Cell offsetHigh, Payload const &payload, primitive::Direction seekDir) {
  assert(payload);

  int const base = _dp.current().offset;
  pushPtr();
  goToDynamicOffset(offsetLow, offsetHigh);
  
  // Base is now the cell we arrived at (at offset).
  // Load values into payload
  for (int i = 0; i != payload.size(); ++i) {
    moveTo(base + i, MacroCell::Value0);
    copyField(Cell{base + i, MacroCell::Payload0}, Temps<1>::select(base + i, MacroCell::Scratch0));
    if (payload.width(i) == Payload::Width::Double) {
      moveTo(base + i, MacroCell::Value1);
      copyField(Cell{base + i, MacroCell::Payload1}, Temps<1>::select(base + i, MacroCell::Scratch0));
    }
  }
  
  // Bring payload back to cell that contains the SeekMarker
  moveTo(base);
  seek(MacroCell::SeekMarker, seekDir, payload, true);
  popPtr();

  // Transfer complete: payload now in Payload-fields of the base
}
