// Add 1 to i16 values without carry into the high byte using add and addAssign
// Expected: ACBCCC

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::i16());
  c.declareLocal("z", ts::i16());

  c.assign("x", literal::i16(CAT('A', 'C')));

  c.write("x");                       // AC
  c.addAssign("x", literal::i16(1));      // x = BC
  c.write("x");                       // BC
  c.assign("z", c.add("x", literal::i16(1))); // z = CC
  c.write("z");                       // CC

  c.returnFromFunction();
} c.endFunction();

TEST_END
