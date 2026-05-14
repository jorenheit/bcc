// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Filename: integer_multiplication_edgecases_u8.cc
// Multiplication edge cases for u8: x*0 and x*1, plus assign variants
// Expected: ABCD

TEST_BEGIN

auto u8 = ts::u8();

c.function("main").begin(); {
  c.declareLocal("x", u8);
  c.declareLocal("y", u8);

  // x * 0 -> 0, mapped to 'A'
  c.assign("x", literal::u8(77));
  c.assign("y", literal::u8(0));
  c.write(c.add(c.mul("x", "y"), literal::u8(65)));

  // 1 * 1 -> 1, mapped to 'B'
  c.assign("x", literal::u8(1));
  c.assign("y", literal::u8(1));
  c.write(c.add(c.mul("x", "y"), literal::u8(65)));

  // x *= 0 -> 0, mapped to 'C'
  c.assign("x", literal::u8(77));
  c.assign("y", literal::u8(0));
  c.mulAssign("x", "y");
  c.write(c.add("x", literal::u8(67)));

  // 1 *= 1 -> 1, mapped to 'D'
  c.assign("x", literal::u8(1));
  c.assign("y", literal::u8(1));
  c.mulAssign("x", "y");
  c.write(c.add("x", literal::u8(67)));

  c.returnFromFunction();
} c.endFunction();

TEST_END
