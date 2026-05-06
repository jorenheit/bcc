// Filename: integer_division_edgecases_i8.cc
// Division edge cases for i8, including division by zero and 0/0
// Expected: ABCD

TEST_BEGIN

auto i8 = ts::i8();

c.function("main").begin(); {
  c.declareLocal("x", i8);
  c.declareLocal("y", i8);

  // x / 0 -> 0xff, mapped to 'A'
  c.assign("x", literal::i8(100));
  c.assign("y", literal::i8(0));
  c.writeOut(c.add(c.div("x", "y"), literal::i8(66)));

  // 0 / 0 -> 0, mapped to 'B'
  c.assign("x", literal::i8(0));
  c.assign("y", literal::i8(0));
  c.writeOut(c.add(c.div("x", "y"), literal::i8(66)));

  // x /= 0 -> 0xff, mapped to 'C'
  c.assign("x", literal::i8(100));
  c.assign("y", literal::i8(0));
  c.divAssign("x", "y");
  c.writeOut(c.add("x", literal::i8(68)));

  // 0 /= 0 -> 0, mapped to 'D'
  c.assign("x", literal::i8(0));
  c.assign("y", literal::i8(0));
  c.divAssign("x", "y");
  c.writeOut(c.add("x", literal::i8(68)));

  c.returnFromFunction();
} c.endFunction();

TEST_END
