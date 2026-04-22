// Filename: integer_modulo_edgecases_i8.cc
// Modulo edge cases for i8: x%0, 0%x, 0%0, and assign variants
// Expected: ABCDE

TEST_BEGIN

auto i8 = TypeSystem::i8();

c.beginFunction("main"); {
  c.declareLocal("x", i8);
  c.declareLocal("y", i8);

  c.beginBlock("entry"); {
    // x % 0 -> 0, mapped to 'A'
    c.assign("x", values::i8(100));
    c.assign("y", values::i8(0));
    c.writeOut(c.add(c.mod("x", "y"), values::i8(65)));

    // 0 % x -> 0, mapped to 'B'
    c.assign("x", values::i8(0));
    c.assign("y", values::i8(7));
    c.writeOut(c.add(c.mod("x", "y"), values::i8(66)));

    // 0 % 0 -> 0, mapped to 'C'
    c.assign("x", values::i8(0));
    c.assign("y", values::i8(0));
    c.writeOut(c.add(c.mod("x", "y"), values::i8(67)));

    // x %= 0 -> 0, mapped to 'D'
    c.assign("x", values::i8(100));
    c.assign("y", values::i8(0));
    c.modAssign("x", "y");
    c.writeOut(c.add("x", values::i8(68)));

    // 0 %= x -> 0, mapped to 'E'
    c.assign("x", values::i8(0));
    c.assign("y", values::i8(7));
    c.modAssign("x", "y");
    c.writeOut(c.add("x", values::i8(69)));

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
