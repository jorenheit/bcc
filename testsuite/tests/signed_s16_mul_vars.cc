// Signed s16 multiplication with variables
// Expected: ABCDEFGH

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::s16());
  c.declareLocal("y", ts::s16());
  c.declareLocal("z", ts::s16());

  // 1. -1 * 2 = -2
  c.assign("x", literal::s16(-1));
  c.assign("y", literal::s16(2));
  c.assign("z", c.mul("x", "y"));
  c.addAssign("z", literal::s16(CAT('C', 'B')));
  c.write("z"); // AB

  // 2. -3 * -2 = 6
  c.assign("x", literal::s16(-3));
  c.assign("y", literal::s16(-2));
  c.assign("z", c.mul("x", "y"));
  c.addAssign("z", literal::s16(CAT('=', 'D')));
  c.write("z"); // CD

  // 3. 300 * 2 = 600
  c.assign("x", literal::s16(300));
  c.assign("y", literal::s16(2));
  c.assign("z", c.mul("x", "y"));
  c.addAssign("z", literal::s16(CAT('E', 'F') - 600));
  c.write("z"); // EF

  // 4. -20 * -10 = 200
  c.assign("x", literal::s16(-20));
  c.assign("y", literal::s16(-10));
  c.assign("z", c.mul("x", "y"));
  c.addAssign("z", literal::s16(CAT('G', 'H') - 200));
  c.write("z"); // GH

  c.returnFromFunction();
} c.endFunction();

TEST_END
