#include "compiler.ih"

Compiler::Mop const Compiler::mulSpec {
  .op = BinOp::Mul,
  .fold = [](int x, int y) -> int { return x * y; },
  .applyWithSlot = &Compiler::mulSlotBySlot,
  .applyWithConst = &Compiler::mulSlotByConst
};

void Compiler::mulSlotByConst(Slot const &lhs, int factor) {

  pushPtr();
  moveTo(lhs, MacroCell::Value0);    
  if (lhs.type->usesValue1()) {
    Slot const tmp = getTemp(TypeSystem::raw(1));
    mul16Const(factor, Cell{lhs, MacroCell::Value1},
	       Temps<8>::select(lhs, MacroCell::Scratch0,
				lhs, MacroCell::Scratch1,
				lhs, MacroCell::Payload0,
				lhs, MacroCell::Payload1,
				tmp, MacroCell::Scratch0,
				tmp, MacroCell::Scratch1,
				tmp, MacroCell::Payload0,
				tmp, MacroCell::Payload1));
  } else {
    mulConst(factor,
	     Temps<3>::select(lhs, MacroCell::Scratch0,
			      lhs, MacroCell::Scratch1,
			      lhs, MacroCell::Payload0));
  }
  
  popPtr();
}


void Compiler::mulSlotBySlot(Slot const &lhs, Slot const &rhs) {
  pushPtr();

  Slot const tmp = getTemp(rhs.type);
  assignSlot(tmp, rhs);  
  moveTo(lhs, MacroCell::Value0);
  if (lhs.type->usesValue1()) {
    mul16Destructive(Cell{lhs, MacroCell::Value1},
		     Cell{tmp, MacroCell::Value0},
		     Cell{tmp, MacroCell::Value1},
		     Temps<9>::select(lhs, MacroCell::Scratch0,
				      lhs, MacroCell::Scratch1,
				      lhs, MacroCell::Payload0,
				      lhs, MacroCell::Payload1,				      
				      tmp, MacroCell::Scratch0,
				      tmp, MacroCell::Scratch1,
				      tmp, MacroCell::Payload0,
				      tmp, MacroCell::Payload1,
				      rhs, MacroCell::Scratch0));
  } else {
    mulDestructive(Cell{tmp, MacroCell::Value0},
		   Temps<3>::select(lhs, MacroCell::Scratch0,
				    lhs, MacroCell::Scratch1,
				    tmp, MacroCell::Scratch0));
  }
  popPtr();
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
