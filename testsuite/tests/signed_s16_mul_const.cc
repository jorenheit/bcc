// Signed s16 multiplication with constants
// Expected: ABCDEFGH

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::s16());
  c.declareLocal("z", ts::s16());

  // 1. -1 * 2 = -2
  // CAT('C', 'B') - 2 = CAT('A', 'B') -> AB
  c.assign("x", literal::s16(-1));
  c.assign("z", c.mul("x", literal::s16(2)));
  c.addAssign("z", literal::s16(CAT('C', 'B')));
  c.writeOut("z"); // AB

  // 2. -3 * -2 = 6
  // CAT('=', 'D') + 6 = CAT('C', 'D') -> CD
  c.assign("x", literal::s16(-3));
  c.assign("z", c.mul("x", literal::s16(-2)));
  c.addAssign("z", literal::s16(CAT('=', 'D')));
  c.writeOut("z"); // CD

  // 3. 300 * 2 = 600
  // CAT('E', 'F') - 600 + 600 = CAT('E', 'F') -> EF
  c.assign("x", literal::s16(300));
  c.assign("z", c.mul("x", literal::s16(2)));
  c.addAssign("z", literal::s16(CAT('E', 'F') - 600));
  c.writeOut("z"); // EF

  // 4. -20 * -10 = 200
  // CAT('G', 'H') - 200 + 200 = CAT('G', 'H') -> GH
  c.assign("x", literal::s16(-20));
  c.assign("z", c.mul("x", literal::s16(-10)));
  c.addAssign("z", literal::s16(CAT('G', 'H') - 200));
  c.writeOut("z"); // GH

  c.returnFromFunction();
} c.endFunction();

TEST_END
