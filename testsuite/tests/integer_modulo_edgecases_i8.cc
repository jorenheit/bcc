// Filename: integer_modulo_edgecases_i8.cc
// Modulo edge cases for i8: x%0, 0%x, 0%0, and assign variants
// Expected: ABCDE

TEST_BEGIN

auto i8 = ts::i8();

c.function("main").begin(); {
  c.declareLocal("x", i8);
  c.declareLocal("y", i8);

  c.block("entry").begin(); {
    // x % 0 -> 0, mapped to 'A'
    c.assign("x", literal::i8(100));
    c.assign("y", literal::i8(0));
    c.writeOut(c.add(c.mod("x", "y"), literal::i8(65)));

    // 0 % x -> 0, mapped to 'B'
    c.assign("x", literal::i8(0));
    c.assign("y", literal::i8(7));
    c.writeOut(c.add(c.mod("x", "y"), literal::i8(66)));

    // 0 % 0 -> 0, mapped to 'C'
    c.assign("x", literal::i8(0));
    c.assign("y", literal::i8(0));
    c.writeOut(c.add(c.mod("x", "y"), literal::i8(67)));

    // x %= 0 -> 0, mapped to 'D'
    c.assign("x", literal::i8(100));
    c.assign("y", literal::i8(0));
    c.modAssign("x", "y");
    c.writeOut(c.add("x", literal::i8(68)));

    // 0 %= x -> 0, mapped to 'E'
    c.assign("x", literal::i8(0));
    c.assign("y", literal::i8(7));
    c.modAssign("x", "y");
    c.writeOut(c.add("x", literal::i8(69)));

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
