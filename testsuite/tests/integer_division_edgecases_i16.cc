// Filename: integer_division_edgecases_i16.cc
// Division edge cases for i16, including division by zero and 0/0
// Expected: AABBCCDD

TEST_BEGIN

auto i16 = TypeSystem::i16();

c.beginFunction("main"); {
  c.declareLocal("x", i16);
  c.declareLocal("y", i16);

  c.beginBlock("entry"); {
    // x / 0 -> 0xffff, mapped to "AA"
    c.assign("x", values::i16(0x1234));
    c.assign("y", values::i16(0));
    c.writeOut(c.add(c.div("x", "y"), values::i16(0x4142)));

    // 0 / 0 -> 0, mapped to "BB"
    c.assign("x", values::i16(0));
    c.assign("y", values::i16(0));
    c.writeOut(c.add(c.div("x", "y"), values::i16(0x4242)));

    // x /= 0 -> 0xffff, mapped to "CC"
    c.assign("x", values::i16(0x1234));
    c.assign("y", values::i16(0));
    c.divAssign("x", "y");
    c.writeOut(c.add("x", values::i16(0x4344)));

    // 0 /= 0 -> 0, mapped to "DD"
    c.assign("x", values::i16(0));
    c.assign("y", values::i16(0));
    c.divAssign("x", "y");
    c.writeOut(c.add("x", values::i16(0x4444)));

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
