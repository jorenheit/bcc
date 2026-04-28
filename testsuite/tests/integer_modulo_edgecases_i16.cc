// Filename: integer_modulo_edgecases_i16.cc
// Modulo edge cases for i16: x%0, 0%x, 0%0, and assign variants
// Expected: AABBCCDDEE

TEST_BEGIN

auto i16 = ts::i16();

c.beginFunction("main"); {
  c.declareLocal("x", i16);
  c.declareLocal("y", i16);

  c.beginBlock("entry"); {
    // x % 0 -> 0, mapped to "AA"
    c.assign("x", literal::i16(0x1234));
    c.assign("y", literal::i16(0));
    c.writeOut(c.add(c.mod("x", "y"), literal::i16(0x4141)));

    // 0 % x -> 0, mapped to "BB"
    c.assign("x", literal::i16(0));
    c.assign("y", literal::i16(0x1111));
    c.writeOut(c.add(c.mod("x", "y"), literal::i16(0x4242)));

    // 0 % 0 -> 0, mapped to "CC"
    c.assign("x", literal::i16(0));
    c.assign("y", literal::i16(0));
    c.writeOut(c.add(c.mod("x", "y"), literal::i16(0x4343)));

    // x %= 0 -> 0, mapped to "DD"
    c.assign("x", literal::i16(0x1234));
    c.assign("y", literal::i16(0));
    c.modAssign("x", "y");
    c.writeOut(c.add("x", literal::i16(0x4444)));

    // 0 %= x -> 0, mapped to "EE"
    c.assign("x", literal::i16(0));
    c.assign("y", literal::i16(0x1111));
    c.modAssign("x", "y");
    c.writeOut(c.add("x", literal::i16(0x4545)));

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
