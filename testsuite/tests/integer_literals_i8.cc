// Filename: integer_literals_i8.cc
// Literal RHS tests for i8 operators and assign-operators
// Expected: ABCDEABCDE

TEST_BEGIN

auto i8 = TypeSystem::i8();

c.beginFunction("main"); {
  c.declareLocal("x", i8);

  c.beginBlock("entry"); {
    // add with literal -> 'A'
    c.assign("x", values::i8(64));
    c.writeOut(c.add("x", values::i8(1)));

    // sub with literal -> 'B'
    c.assign("x", values::i8(67));
    c.writeOut(c.sub("x", values::i8(1)));

    // div with literal, mapped to 'C'
    c.assign("x", values::i8(99));
    c.writeOut(c.add(c.div("x", values::i8(3)), values::i8(34)));

    // mul with literal -> 'D'
    c.assign("x", values::i8(34));
    c.writeOut(c.mul("x", values::i8(2)));

    // mod with literal, mapped to 'E'
    c.assign("x", values::i8(100));
    c.writeOut(c.add(c.mod("x", values::i8(34)), values::i8(37)));

    // addAssign with literal -> 'A'
    c.assign("x", values::i8(64));
    c.addAssign("x", values::i8(1));
    c.writeOut("x");

    // subAssign with literal -> 'B'
    c.assign("x", values::i8(67));
    c.subAssign("x", values::i8(1));
    c.writeOut("x");

    // divAssign with literal, mapped to 'C'
    c.assign("x", values::i8(99));
    c.divAssign("x", values::i8(3));
    c.writeOut(c.add("x", values::i8(34)));

    // mulAssign with literal -> 'D'
    c.assign("x", values::i8(34));
    c.mulAssign("x", values::i8(2));
    c.writeOut("x");

    // modAssign with literal, mapped to 'E'
    c.assign("x", values::i8(100));
    c.modAssign("x", values::i8(34));
    c.writeOut(c.add("x", values::i8(37)));

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
