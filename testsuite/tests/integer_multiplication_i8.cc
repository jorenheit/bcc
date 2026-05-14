// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Filename: integer_multiplication_u8.cc
// Multiply u8 integers using mul and mulAssign
// Expected: AM

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::u8());
  c.declareLocal("y", ts::u8());
  c.declareLocal("z", ts::u8());

  c.assign("x", literal::u8(5));
  c.assign("y", literal::u8(13));

  c.mulAssign("x", "y");           // 5 * 13 = 65 -> A
  c.write("x");                 // A
  c.assign("z", c.mul("x", "y"));  // 65 * 13 = 845 mod 256 = 77 -> M
  c.write("z");                 // M

  c.returnFromFunction();
} c.endFunction();

TEST_END
