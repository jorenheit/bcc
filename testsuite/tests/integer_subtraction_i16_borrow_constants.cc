// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Subtract u16 constants with borrow into the high byte
// Expected: ACCBEA

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::u16());
  c.declareLocal("z", ts::u16());

  c.assign("x", literal::u16(CAT('A', 'C')));

  c.write("x");                              // AC
  c.subAssign("x", literal::u16(254));
  c.write("x");                              // CB
  c.assign("z", c.sub("x", literal::u16(254)));
  c.write("z");                              // EA

  c.returnFromFunction();
} c.endFunction();

TEST_END
