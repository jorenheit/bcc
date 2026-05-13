// Filename: integer_multiplication_edgecases_i16.cc
// Multiplication edge cases for i16: x*0 and x*1, plus assign variants
// Expected: AABBCCDD

TEST_BEGIN

auto i16 = ts::i16();

c.function("main").begin(); {
  c.declareLocal("x", i16);
  c.declareLocal("y", i16);

  // x * 0 -> 0, mapped to "AA"
  c.assign("x", literal::i16(0x1234));
  c.assign("y", literal::i16(0));
  c.write(c.add(c.mul("x", "y"), literal::i16(0x4141)));

  // 1 * 1 -> 1, mapped to "BB"
  c.assign("x", literal::i16(1));
  c.assign("y", literal::i16(1));
  c.write(c.add(c.mul("x", "y"), literal::i16(0x4241)));

  // x *= 0 -> 0, mapped to "CC"
  c.assign("x", literal::i16(0x1234));
  c.assign("y", literal::i16(0));
  c.mulAssign("x", "y");
  c.write(c.add("x", literal::i16(0x4343)));

  // 1 *= 1 -> 1, mapped to "DD"
  c.assign("x", literal::i16(1));
  c.assign("y", literal::i16(1));
  c.mulAssign("x", "y");
  c.write(c.add("x", literal::i16(0x4443)));

  c.returnFromFunction();
} c.endFunction();

TEST_END
