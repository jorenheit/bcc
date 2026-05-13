// Add 1 to an i16 value without carry into the high byte
// Expected: ACBC

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::i16());

  c.assign("x", literal::i16(CAT('A', 'C')));

  c.write("x");                  // AC
  c.addAssign("x", literal::i16(1)); // should become BC
  c.write("x");                  // BC

  c.returnFromFunction();
} c.endFunction();

TEST_END
