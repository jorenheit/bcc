// Filename: integer_multiplication_edgecases_i8.cc
// Multiplication edge cases for i8: x*0 and x*1, plus assign variants
// Expected: ABCD

TEST_BEGIN

auto i8 = TypeSystem::i8();

c.beginFunction("main"); {
  c.declareLocal("x", i8);
  c.declareLocal("y", i8);

  c.beginBlock("entry"); {
    // x * 0 -> 0, mapped to 'A'
    c.assign("x", values::i8(77));
    c.assign("y", values::i8(0));
    c.writeOut(c.add(c.mul("x", "y"), values::i8(65)));

    // 1 * 1 -> 1, mapped to 'B'
    c.assign("x", values::i8(1));
    c.assign("y", values::i8(1));
    c.writeOut(c.add(c.mul("x", "y"), values::i8(65)));

    // x *= 0 -> 0, mapped to 'C'
    c.assign("x", values::i8(77));
    c.assign("y", values::i8(0));
    c.mulAssign("x", "y");
    c.writeOut(c.add("x", values::i8(67)));

    // 1 *= 1 -> 1, mapped to 'D'
    c.assign("x", values::i8(1));
    c.assign("y", values::i8(1));
    c.mulAssign("x", "y");
    c.writeOut(c.add("x", values::i8(67)));

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
