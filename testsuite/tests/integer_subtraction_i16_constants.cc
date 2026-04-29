// Subtract i16 constants using sub and subAssign
// Expected: AGADAA

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::i16());
  c.declareLocal("z", ts::i16());

  c.block("entry").begin(); {
    c.assign("x", literal::i16(CAT('G', 'A')));

    c.writeOut("x");                           // GA
    c.subAssign("x", literal::i16(3));          // x -= 3
    c.writeOut("x");                           // DA
    c.assign("z", c.sub("x", literal::i16(3))); // z = x - 3
    c.writeOut("z");                           // AA

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
