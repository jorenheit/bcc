// Signed widening s8 -> s16 must sign-extend
// Expected: ABBDBA

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::s8());
  c.declareLocal("y", ts::s16());

  // -1 sign-extended to s16 is 0xffff.
  // 0xffff + 0x4242 = 0x4241 -> AB.
  // If this zero-extends instead, result becomes 0x4341 -> AC.
  c.assign("x", literal::s8(-1));
  c.assign("y", "x");
  c.addAssign("y", literal::s16(CAT('B', 'B')));
  c.write("y"); // AB

  // -2 sign-extended to s16 is 0xfffe.
  // 0xfffe + 0x4444 = 0x4442 -> BD.
  // If this zero-extends instead, result becomes 0x4542 -> BE.
  c.assign("x", literal::s8(-2));
  c.assign("y", "x");
  c.addAssign("y", literal::s16(CAT('D', 'D')));
  c.write("y"); // BD

  // Positive widening should still zero-extend normally.
  // 1 + 0x4141 = 0x4142 -> BA.
  c.assign("x", literal::s8(1));
  c.assign("y", "x");
  c.addAssign("y", literal::s16(CAT('A', 'A')));
  c.write("y"); // BA

  c.returnFromFunction();
} c.endFunction();

TEST_END
