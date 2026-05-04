// Signed s16 division and modulo with slot/slot operands
// Expected: ABCDEFGHIJKLMNOP

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::s16());
  c.declareLocal("y", ts::s16());
  c.declareLocal("z", ts::s16());

  c.block("entry").begin(); {
    // -7 / 3 = -2; CAT('C', 'B') - 2 = CAT('A', 'B') -> AB
    c.assign("x", literal::s16(-7));
    c.assign("y", literal::s16(3));
    c.assign("z", c.div("x", "y"));
    c.addAssign("z", literal::s16(CAT('C', 'B')));
    c.writeOut("z"); // AB

    // 7 / -3 = -2; CAT('E', 'D') - 2 = CAT('C', 'D') -> CD
    c.assign("x", literal::s16(7));
    c.assign("y", literal::s16(-3));
    c.assign("z", c.div("x", "y"));
    c.addAssign("z", literal::s16(CAT('E', 'D')));
    c.writeOut("z"); // CD

    // -7 / -3 = 2; CAT('C', 'F') + 2 = CAT('E', 'F') -> EF
    c.assign("x", literal::s16(-7));
    c.assign("y", literal::s16(-3));
    c.assign("z", c.div("x", "y"));
    c.addAssign("z", literal::s16(CAT('C', 'F')));
    c.writeOut("z"); // EF

    // 7 / 3 = 2; CAT('E', 'H') + 2 = CAT('G', 'H') -> GH
    c.assign("x", literal::s16(7));
    c.assign("y", literal::s16(3));
    c.assign("z", c.div("x", "y"));
    c.addAssign("z", literal::s16(CAT('E', 'H')));
    c.writeOut("z"); // GH

    // -7 % 3 = -1; CAT('J', 'J') - 1 = CAT('I', 'J') -> IJ
    c.assign("x", literal::s16(-7));
    c.assign("y", literal::s16(3));
    c.assign("z", c.mod("x", "y"));
    c.addAssign("z", literal::s16(CAT('J', 'J')));
    c.writeOut("z"); // IJ

    // 7 % -3 = 1; CAT('J', 'L') + 1 = CAT('K', 'L') -> KL
    c.assign("x", literal::s16(7));
    c.assign("y", literal::s16(-3));
    c.assign("z", c.mod("x", "y"));
    c.addAssign("z", literal::s16(CAT('J', 'L')));
    c.writeOut("z"); // KL

    // -7 % -3 = -1; CAT('N', 'N') - 1 = CAT('M', 'N') -> MN
    c.assign("x", literal::s16(-7));
    c.assign("y", literal::s16(-3));
    c.assign("z", c.mod("x", "y"));
    c.addAssign("z", literal::s16(CAT('N', 'N')));
    c.writeOut("z"); // MN

    // 7 % 3 = 1; CAT('N', 'P') + 1 = CAT('O', 'P') -> OP
    c.assign("x", literal::s16(7));
    c.assign("y", literal::s16(3));
    c.assign("z", c.mod("x", "y"));
    c.addAssign("z", literal::s16(CAT('N', 'P')));
    c.writeOut("z"); // OP

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
