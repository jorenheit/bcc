// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Add 1 to u16 values without carry into the high byte using add and addAssign
// Expected: ACBCCC

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::u16());
  c.declareLocal("z", ts::u16());

  c.assign("x", literal::u16(CAT('A', 'C')));

  c.write("x");                       // AC
  c.addAssign("x", literal::u16(1));      // x = BC
  c.write("x");                       // BC
  c.assign("z", c.add("x", literal::u16(1))); // z = CC
  c.write("z");                       // CC

  c.returnFromFunction();
} c.endFunction();

TEST_END
