// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Filename: integer_modulo_edgecases_u8.cc
// Modulo edge cases for u8: x%0, 0%x, 0%0, and assign variants
// Expected: ABCDE

TEST_BEGIN

auto u8 = ts::u8();

c.function("main").begin(); {
  c.declareLocal("x", u8);
  c.declareLocal("y", u8);

  // x % 0 -> 0, mapped to 'A'
  c.assign("x", literal::u8(100));
  c.assign("y", literal::u8(0));
  c.write(c.add(c.mod("x", "y"), literal::u8(65)));

  // 0 % x -> 0, mapped to 'B'
  c.assign("x", literal::u8(0));
  c.assign("y", literal::u8(7));
  c.write(c.add(c.mod("x", "y"), literal::u8(66)));

  // 0 % 0 -> 0, mapped to 'C'
  c.assign("x", literal::u8(0));
  c.assign("y", literal::u8(0));
  c.write(c.add(c.mod("x", "y"), literal::u8(67)));

  // x %= 0 -> 0, mapped to 'D'
  c.assign("x", literal::u8(100));
  c.assign("y", literal::u8(0));
  c.modAssign("x", "y");
  c.write(c.add("x", literal::u8(68)));

  // 0 %= x -> 0, mapped to 'E'
  c.assign("x", literal::u8(0));
  c.assign("y", literal::u8(7));
  c.modAssign("x", "y");
  c.write(c.add("x", literal::u8(69)));

  c.returnFromFunction();
} c.endFunction();

TEST_END
