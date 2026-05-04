// Signed s16 division and modulo with slot/const and const/slot operands
// Expected: ABCDEFGHIJKLMNOP

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::s16());
  c.declareLocal("z", ts::s16());

  c.block("entry").begin(); {
    // slot / const: -12 / 5 = -2 -> AB
    c.assign("x", literal::s16(-12));
    c.assign("z", c.div("x", literal::s16(5)));
    c.addAssign("z", literal::s16(CAT('C', 'B')));
    c.writeOut("z"); // AB

    // slot / const: 12 / -5 = -2 -> CD
    c.assign("x", literal::s16(12));
    c.assign("z", c.div("x", literal::s16(-5)));
    c.addAssign("z", literal::s16(CAT('E', 'D')));
    c.writeOut("z"); // CD

    // const / slot: -12 / 5 = -2 -> EF
    c.assign("x", literal::s16(5));
    c.assign("z", c.div(literal::s16(-12), "x"));
    c.addAssign("z", literal::s16(CAT('G', 'F')));
    c.writeOut("z"); // EF

    // const / slot: 12 / -5 = -2 -> GH
    c.assign("x", literal::s16(-5));
    c.assign("z", c.div(literal::s16(12), "x"));
    c.addAssign("z", literal::s16(CAT('I', 'H')));
    c.writeOut("z"); // GH

    // slot % const: -12 % 5 = -2 -> IJ
    c.assign("x", literal::s16(-12));
    c.assign("z", c.mod("x", literal::s16(5)));
    c.addAssign("z", literal::s16(CAT('K', 'J')));
    c.writeOut("z"); // IJ

    // slot % const: 12 % -5 = 2 -> KL
    c.assign("x", literal::s16(12));
    c.assign("z", c.mod("x", literal::s16(-5)));
    c.addAssign("z", literal::s16(CAT('I', 'L')));
    c.writeOut("z"); // KL

    // const % slot: -12 % 5 = -2 -> MN
    c.assign("x", literal::s16(5));
    c.assign("z", c.mod(literal::s16(-12), "x"));
    c.addAssign("z", literal::s16(CAT('O', 'N')));
    c.writeOut("z"); // MN

    // const % slot: 12 % -5 = 2 -> OP
    c.assign("x", literal::s16(-5));
    c.assign("z", c.mod(literal::s16(12), "x"));
    c.addAssign("z", literal::s16(CAT('M', 'P')));
    c.writeOut("z"); // OP

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
