// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Signed div/mod literal folding
// Expected: ABCDEFGHIJKLMNOP

TEST_BEGIN

c.function("main").begin(); {
  // s8 literal folding

  // -7 / 3 = -2; -2 + 'C' = 'A'
  c.write(c.add(c.div(literal::s8(-7), literal::s8(3)),
  literal::s8('C'))); // A

  // 7 / -3 = -2; -2 + 'D' = 'B'
  c.write(c.add(c.div(literal::s8(7), literal::s8(-3)),
  literal::s8('D'))); // B

  // -7 / -3 = 2; 2 + 'A' = 'C'
  c.write(c.add(c.div(literal::s8(-7), literal::s8(-3)),
  literal::s8('A'))); // C

  // 7 / 3 = 2; 2 + 'B' = 'D'
  c.write(c.add(c.div(literal::s8(7), literal::s8(3)),
  literal::s8('B'))); // D

  // -7 % 3 = -1; -1 + 'F' = 'E'
  c.write(c.add(c.mod(literal::s8(-7), literal::s8(3)),
  literal::s8('F'))); // E

  // 7 % -3 = 1; 1 + 'E' = 'F'
  c.write(c.add(c.mod(literal::s8(7), literal::s8(-3)),
  literal::s8('E'))); // F

  // -7 % -3 = -1; -1 + 'H' = 'G'
  c.write(c.add(c.mod(literal::s8(-7), literal::s8(-3)),
  literal::s8('H'))); // G

  // 7 % 3 = 1; 1 + 'G' = 'H'
  c.write(c.add(c.mod(literal::s8(7), literal::s8(3)),
  literal::s8('G'))); // H

  // s16 literal folding

  // -7 / 3 = -2 -> IJ
  c.write(c.add(c.div(literal::s16(-7), literal::s16(3)),
  literal::s16(CAT('K', 'J')))); // IJ

  // 7 / -3 = -2 -> KL
  c.write(c.add(c.div(literal::s16(7), literal::s16(-3)),
  literal::s16(CAT('M', 'L')))); // KL

  // -7 % 3 = -1 -> MN
  c.write(c.add(c.mod(literal::s16(-7), literal::s16(3)),
  literal::s16(CAT('N', 'N')))); // MN

  // 7 % -3 = 1 -> OP
  c.write(c.add(c.mod(literal::s16(7), literal::s16(-3)),
  literal::s16(CAT('N', 'P')))); // OP

  c.returnFromFunction();
} c.endFunction();

TEST_END
