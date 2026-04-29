// Subtract i16 constants with borrow into the high byte
// Expected: ACCBEA

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::i16());
  c.declareLocal("z", ts::i16());

  c.block("entry").begin(); {
    c.assign("x", literal::i16(CAT('A', 'C'))); 

    c.writeOut("x");                              // AC
    c.subAssign("x", literal::i16(254));   
    c.writeOut("x");                              // CB
    c.assign("z", c.sub("x", literal::i16(254)));
    c.writeOut("z");                              // EA

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
