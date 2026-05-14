// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Signed s16 unary sanity: negate, abs, signBit
// Expected: ABCDEFGHIJKLMNOPQQRRSS

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::s16());
  c.declareLocal("z", ts::s16());
  c.declareLocal("f", ts::s16());


  // negate(-1) = 1; CAT('@', 'B') + 1 = CAT('A', 'B') -> AB
  c.assign("x", literal::s16(-1));
  c.assign("z", c.negate("x"));
  c.addAssign("z", literal::s16(CAT('@', 'B')));
  c.write("z"); // AB

  // negate(2) = -2; CAT('E', 'D') - 2 = CAT('C', 'D') -> CD
  c.assign("x", literal::s16(2));
  c.assign("z", c.negate("x"));
  c.addAssign("z", literal::s16(CAT('E', 'D')));
  c.write("z"); // CD

  // negateAssign(-3) = 3; CAT('B', 'F') + 3 = CAT('E', 'F') -> EF
  c.assign("x", literal::s16(-3));
  c.negateAssign("x");
  c.addAssign("x", literal::s16(CAT('B', 'F')));
  c.write("x"); // EF

  // negateAssign(4) = -4; CAT('K', 'H') - 4 = CAT('G', 'H') -> GH
  c.assign("x", literal::s16(4));
  c.negateAssign("x");
  c.addAssign("x", literal::s16(CAT('K', 'H')));
  c.write("x"); // GH

  // abs(-5) = 5; CAT('D', 'J') + 5 = CAT('I', 'J') -> IJ
  c.assign("x", literal::s16(-5));
  c.assign("z", c.abs("x"));
  c.addAssign("z", literal::s16(CAT('D', 'J')));
  c.write("z"); // IJ

  // abs(6) = 6; CAT('E', 'L') + 6 = CAT('K', 'L') -> KL
  c.assign("x", literal::s16(6));
  c.assign("z", c.abs("x"));
  c.addAssign("z", literal::s16(CAT('E', 'L')));
  c.write("z"); // KL

  // absAssign(-7) = 7; CAT('F', 'N') + 7 = CAT('M', 'N') -> MN
  c.assign("x", literal::s16(-7));
  c.absAssign("x");
  c.addAssign("x", literal::s16(CAT('F', 'N')));
  c.write("x"); // MN

  // absAssign(8) = 8; CAT('G', 'P') + 8 = CAT('O', 'P') -> OP
  c.assign("x", literal::s16(8));
  c.absAssign("x");
  c.addAssign("x", literal::s16(CAT('G', 'P')));
  c.write("x"); // OP

  // ORRU

  // signBit(-1) = 1; 1 + 'P' = 'Q'
  c.assign("x", literal::s16(-1));
  c.assign("f", c.cast(c.signBit("x"), ts::s16()));
  c.write(c.add("f", literal::s16(CAT('P', 'Q')))); // QQ

  // signBit(0) = 0; 0 + 'R' = 'R'
  c.assign("x", literal::s16(0));
  c.assign("f", c.cast(c.signBit("x"), ts::s16()));
  c.write(c.add("f", literal::s16(CAT('R', 'R')))); // RR

  // signBitAssign(-32768): x becomes 1; CAT('R', 'T') + 1 = CAT('S', 'T') -> SS
  c.assign("x", literal::s16(-32768));
  c.signBitAssign("x");
  c.addAssign("x", literal::s16(CAT('R', 'S')));
  c.write("x"); // ST

  c.returnFromFunction();
} c.endFunction();

TEST_END
