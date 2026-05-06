// Signed s16 divAssign and modAssign
// Expected: ABCDEFGHIJKLMNOP

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::s16());
  c.declareLocal("y", ts::s16());

  // -7 / 3 = -2 -> AB
  c.assign("x", literal::s16(-7));
  c.assign("y", literal::s16(3));
  c.divAssign("x", "y");
  c.addAssign("x", literal::s16(CAT('C', 'B')));
  c.writeOut("x"); // AB

  // 7 / -3 = -2 -> CD
  c.assign("x", literal::s16(7));
  c.assign("y", literal::s16(-3));
  c.divAssign("x", "y");
  c.addAssign("x", literal::s16(CAT('E', 'D')));
  c.writeOut("x"); // CD

  // -7 / -3 = 2 -> EF
  c.assign("x", literal::s16(-7));
  c.assign("y", literal::s16(-3));
  c.divAssign("x", "y");
  c.addAssign("x", literal::s16(CAT('C', 'F')));
  c.writeOut("x"); // EF

  // 7 / 3 = 2 -> GH
  c.assign("x", literal::s16(7));
  c.assign("y", literal::s16(3));
  c.divAssign("x", "y");
  c.addAssign("x", literal::s16(CAT('E', 'H')));
  c.writeOut("x"); // GH

  // -7 % 3 = -1 -> IJ
  c.assign("x", literal::s16(-7));
  c.assign("y", literal::s16(3));
  c.modAssign("x", "y");
  c.addAssign("x", literal::s16(CAT('J', 'J')));
  c.writeOut("x"); // IJ

  // 7 % -3 = 1 -> KL
  c.assign("x", literal::s16(7));
  c.assign("y", literal::s16(-3));
  c.modAssign("x", "y");
  c.addAssign("x", literal::s16(CAT('J', 'L')));
  c.writeOut("x"); // KL

  // -7 % -3 = -1 -> MN
  c.assign("x", literal::s16(-7));
  c.assign("y", literal::s16(-3));
  c.modAssign("x", "y");
  c.addAssign("x", literal::s16(CAT('N', 'N')));
  c.writeOut("x"); // MN

  // 7 % 3 = 1 -> OP
  c.assign("x", literal::s16(7));
  c.assign("y", literal::s16(3));
  c.modAssign("x", "y");
  c.addAssign("x", literal::s16(CAT('N', 'P')));
  c.writeOut("x"); // OP

  c.returnFromFunction();
} c.endFunction();

TEST_END
