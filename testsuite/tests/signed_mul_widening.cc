// Signed mixed-width multiplication: s8 with s16 promotes to s16
// Expected: ABCDEFGH

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("a", ts::s8());
  c.declareLocal("b", ts::s16());
  c.declareLocal("z", ts::s16());

  // 1. s8(-3) * s16(2) = -6
  // CAT('G', 'B') - 6 = CAT('A', 'B') -> AB
  c.assign("a", literal::s8(-3));
  c.assign("b", literal::s16(2));
  c.assign("z", c.mul("a", "b"));
  c.addAssign("z", literal::s16(CAT('G', 'B')));
  c.write("z"); // AB

  // 2. s16(5) * s8(-4) = -20
  // CAT('W', 'D') - 20 = CAT('C', 'D') -> CD
  c.assign("a", literal::s8(-4));
  c.assign("b", literal::s16(5));
  c.assign("z", c.mul("b", "a"));
  c.addAssign("z", literal::s16(CAT('W', 'D')));
  c.write("z"); // CD

  // 3. s8(-6) * s16(-10) = 60
  // CAT('E', 'F') - 60 + 60 = CAT('E', 'F') -> EF
  c.assign("a", literal::s8(-6));
  c.assign("b", literal::s16(-10));
  c.assign("z", c.mul("a", "b"));
  c.addAssign("z", literal::s16(CAT('E', 'F') - 60));
  c.write("z"); // EF

  // 4. s16(100) * s8(3) = 300
  // CAT('G', 'H') - 300 + 300 = CAT('G', 'H') -> GH
  c.assign("a", literal::s8(3));
  c.assign("b", literal::s16(100));
  c.assign("z", c.mul("b", "a"));
  c.addAssign("z", literal::s16(CAT('G', 'H') - 300));
  c.write("z"); // GH

  c.returnFromFunction();
} c.endFunction();

TEST_END
