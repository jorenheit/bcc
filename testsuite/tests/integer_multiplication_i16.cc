// Filename: integer_multiplication_i16.cc
// Multiply i16 integers using mul and mulAssign
// Expected: ""DD

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::i16());
  c.declareLocal("y", ts::i16());
  c.declareLocal("z", ts::i16());

  c.assign("x", literal::i16(0x1111));
  c.assign("y", literal::i16(2));

  c.mulAssign("x", "y");           // 0x1111 * 2 = 0x2222 -> ""
  c.writeOut("x");                 // ""
  c.assign("z", c.mul("x", "y"));  // 0x2222 * 2 = 0x4444 -> DD
  c.writeOut("z");                 // DD

  c.returnFromFunction();
} c.endFunction();

TEST_END
