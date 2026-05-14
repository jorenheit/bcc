// Filename: integer_multiplication_edgecases_u16.cc
// Multiplication edge cases for u16: x*0 and x*1, plus assign variants
// Expected: AABBCCDD

TEST_BEGIN

auto u16 = ts::u16();

c.function("main").begin(); {
  c.declareLocal("x", u16);
  c.declareLocal("y", u16);

  // x * 0 -> 0, mapped to "AA"
  c.assign("x", literal::u16(0x1234));
  c.assign("y", literal::u16(0));
  c.write(c.add(c.mul("x", "y"), literal::u16(0x4141)));

  // 1 * 1 -> 1, mapped to "BB"
  c.assign("x", literal::u16(1));
  c.assign("y", literal::u16(1));
  c.write(c.add(c.mul("x", "y"), literal::u16(0x4241)));

  // x *= 0 -> 0, mapped to "CC"
  c.assign("x", literal::u16(0x1234));
  c.assign("y", literal::u16(0));
  c.mulAssign("x", "y");
  c.write(c.add("x", literal::u16(0x4343)));

  // 1 *= 1 -> 1, mapped to "DD"
  c.assign("x", literal::u16(1));
  c.assign("y", literal::u16(1));
  c.mulAssign("x", "y");
  c.write(c.add("x", literal::u16(0x4443)));

  c.returnFromFunction();
} c.endFunction();

TEST_END
