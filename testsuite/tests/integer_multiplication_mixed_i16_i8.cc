// Filename: integer_multiplication_mixed_i16_i8.cc
// Multiply mixed i16/i8 integers using mul and mulAssign
// Expected: ""DD

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::i16());
  c.declareLocal("y", ts::i8());
  c.declareLocal("z", ts::i16());

  c.assign("x", literal::i16(0x1111));
  c.assign("y", literal::i8(2));

  c.mulAssign("x", "y");           // 0x1111 * 2 = 0x2222 -> ""
  c.write("x");                 // ""
  c.assign("z", c.mul("x", "y"));  // 0x2222 * 2 = 0x4444 -> DD
  c.write("z");                 // DD

  c.returnFromFunction();
} c.endFunction();

TEST_END
