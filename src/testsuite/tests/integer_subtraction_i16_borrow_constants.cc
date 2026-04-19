// Subtract i16 constants with borrow into the high byte
// Expected: ACCBEA

TEST_BEGIN

c.beginFunction("main"); {
  c.declareLocal("x", TypeSystem::i16());
  c.declareLocal("z", TypeSystem::i16());

  c.beginBlock("entry"); {
    c.assign("x", values::i16(CAT('A', 'C'))); 

    c.writeOut("x");                              // AC
    c.subAssign("x", values::i16(254));   
    c.writeOut("x");                              // CB
    c.assign("z", c.sub("x", values::i16(254)));
    c.writeOut("z");                              // EA

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
