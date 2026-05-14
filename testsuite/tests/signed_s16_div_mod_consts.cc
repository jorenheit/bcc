// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Signed s16 division and modulo with slot/const and const/slot operands
// Expected: ABCDEFGHIJKLMNOP

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::s16());
  c.declareLocal("z", ts::s16());

  // slot / const: -12 / 5 = -2 -> AB
  c.assign("x", literal::s16(-12));
  c.assign("z", c.div("x", literal::s16(5)));
  c.addAssign("z", literal::s16(CAT('C', 'B')));
  c.write("z"); // AB

  // slot / const: 12 / -5 = -2 -> CD
  c.assign("x", literal::s16(12));
  c.assign("z", c.div("x", literal::s16(-5)));
  c.addAssign("z", literal::s16(CAT('E', 'D')));
  c.write("z"); // CD

  // const / slot: -12 / 5 = -2 -> EF
  c.assign("x", literal::s16(5));
  c.assign("z", c.div(literal::s16(-12), "x"));
  c.addAssign("z", literal::s16(CAT('G', 'F')));
  c.write("z"); // EF

  // const / slot: 12 / -5 = -2 -> GH
  c.assign("x", literal::s16(-5));
  c.assign("z", c.div(literal::s16(12), "x"));
  c.addAssign("z", literal::s16(CAT('I', 'H')));
  c.write("z"); // GH

  // slot % const: -12 % 5 = -2 -> IJ
  c.assign("x", literal::s16(-12));
  c.assign("z", c.mod("x", literal::s16(5)));
  c.addAssign("z", literal::s16(CAT('K', 'J')));
  c.write("z"); // IJ

  // slot % const: 12 % -5 = 2 -> KL
  c.assign("x", literal::s16(12));
  c.assign("z", c.mod("x", literal::s16(-5)));
  c.addAssign("z", literal::s16(CAT('I', 'L')));
  c.write("z"); // KL

  // const % slot: -12 % 5 = -2 -> MN
  c.assign("x", literal::s16(5));
  c.assign("z", c.mod(literal::s16(-12), "x"));
  c.addAssign("z", literal::s16(CAT('O', 'N')));
  c.write("z"); // MN

  // const % slot: 12 % -5 = 2 -> OP
  c.assign("x", literal::s16(-5));
  c.assign("z", c.mod(literal::s16(12), "x"));
  c.addAssign("z", literal::s16(CAT('M', 'P')));
  c.write("z"); // OP

  c.returnFromFunction();
} c.endFunction();

TEST_END
