// Filename: integer_multiplication_edgecases_i8.cc
// Multiplication edge cases for i8: x*0 and x*1, plus assign variants
// Expected: ABCD

TEST_BEGIN

auto i8 = ts::i8();

c.function("main").begin(); {
  c.declareLocal("x", i8);
  c.declareLocal("y", i8);

  c.block("entry").begin(); {
    // x * 0 -> 0, mapped to 'A'
    c.assign("x", literal::i8(77));
    c.assign("y", literal::i8(0));
    c.writeOut(c.add(c.mul("x", "y"), literal::i8(65)));

    // 1 * 1 -> 1, mapped to 'B'
    c.assign("x", literal::i8(1));
    c.assign("y", literal::i8(1));
    c.writeOut(c.add(c.mul("x", "y"), literal::i8(65)));

    // x *= 0 -> 0, mapped to 'C'
    c.assign("x", literal::i8(77));
    c.assign("y", literal::i8(0));
    c.mulAssign("x", "y");
    c.writeOut(c.add("x", literal::i8(67)));

    // 1 *= 1 -> 1, mapped to 'D'
    c.assign("x", literal::i8(1));
    c.assign("y", literal::i8(1));
    c.mulAssign("x", "y");
    c.writeOut(c.add("x", literal::i8(67)));

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
