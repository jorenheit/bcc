// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Signed s8 multiplication with constants
// Expected: ABCDEF

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::s8());
  c.declareLocal("z", ts::s8());

  // 1. -3 * 2 = -6; -6 + 'G' = 'A'
  c.assign("x", literal::s8(-3));
  c.assign("z", c.mul("x", literal::s8(2)));
  c.write(c.add("z", literal::s8('G'))); // A

  // 2. 7 * -1 = -7; -7 + 'I' = 'B'
  c.assign("x", literal::s8(7));
  c.assign("z", c.mul("x", literal::s8(-1)));
  c.write(c.add("z", literal::s8('I'))); // B

  // 3. -4 * -4 = 16; 16 + '3' = 'C'
  c.assign("x", literal::s8(-4));
  c.assign("z", c.mul("x", literal::s8(-4)));
  c.write(c.add("z", literal::s8('3'))); // C

  // 4. 9 * -5 = -45; -45 + 'q' = 'D'
  c.assign("x", literal::s8(9));
  c.assign("z", c.mul("x", literal::s8(-5)));
  c.write(c.add("z", literal::s8('q'))); // D

  // 5. -11 * -3 = 33; 33 + '$' = 'E'
  c.assign("x", literal::s8(-11));
  c.assign("z", c.mul("x", literal::s8(-3)));
  c.write(c.add("z", literal::s8('$'))); // E

  // 6. 12 * -2 = -24; -24 + '^' = 'F'
  c.assign("x", literal::s8(12));
  c.assign("z", c.mul("x", literal::s8(-2)));
  c.write(c.add("z", literal::s8('^'))); // F

  c.returnFromFunction();
} c.endFunction();

TEST_END
