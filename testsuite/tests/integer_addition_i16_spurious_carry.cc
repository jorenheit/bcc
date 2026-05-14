// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Add 1 to an u16 value without carry into the high byte
// Expected: ACBC

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::u16());

  c.assign("x", literal::u16(CAT('A', 'C')));

  c.write("x");                  // AC
  c.addAssign("x", literal::u16(1)); // should become BC
  c.write("x");                  // BC

  c.returnFromFunction();
} c.endFunction();

TEST_END
