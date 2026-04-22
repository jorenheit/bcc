// Filename: integer_literals_i16.cc
// Literal RHS tests for i16 operators and assign-operators
// Expected: AABBCCDDEEAABBCCDDEE

TEST_BEGIN

auto i16 = TypeSystem::i16();

c.beginFunction("main"); {
  c.declareLocal("x", i16);

  c.beginBlock("entry"); {
    // add with literal -> "AA"
    c.assign("x", values::i16(0x4140));
    c.writeOut(c.add("x", values::i16(1)));

    // sub with literal -> "BB"
    c.assign("x", values::i16(0x4243));
    c.writeOut(c.sub("x", values::i16(1)));

    // mul with literal, mapped to "CC"
    c.assign("x", values::i16(0x1111));
    c.writeOut(c.add(c.mul("x", values::i16(3)), values::i16(0x1010)));

    // div with literal, mapped to "DD"
    c.assign("x", values::i16(0x6363));
    c.writeOut(c.add(c.div("x", values::i16(3)), values::i16(0x2323)));

    // mod with literal, mapped to "EE"
    c.assign("x", values::i16(0x7171));
    c.writeOut(c.add(c.mod("x", values::i16(0x5050)), values::i16(0x2424)));

    // addAssign with literal -> "AA"
    c.assign("x", values::i16(0x4140));
    c.addAssign("x", values::i16(1));
    c.writeOut("x");

    // subAssign with literal -> "BB"
    c.assign("x", values::i16(0x4243));
    c.subAssign("x", values::i16(1));
    c.writeOut("x");

    // mulAssign with literal, mapped to "CC"
    c.assign("x", values::i16(0x1111));
    c.mulAssign("x", values::i16(3));
    c.writeOut(c.add("x", values::i16(0x1010)));

    // divAssign with literal, mapped to "DD"
    c.assign("x", values::i16(0x6363));
    c.divAssign("x", values::i16(3));
    c.writeOut(c.add("x", values::i16(0x2323)));

    // modAssign with literal, mapped to "EE"
    c.assign("x", values::i16(0x7171));
    c.modAssign("x", values::i16(0x5050));
    c.writeOut(c.add("x", values::i16(0x2424)));

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
