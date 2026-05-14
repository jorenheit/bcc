// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Filename: integer_multiplication_u16.cc
// Multiply u16 integers using mul and mulAssign
// Expected: ""DD

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::u16());
  c.declareLocal("y", ts::u16());
  c.declareLocal("z", ts::u16());

  c.assign("x", literal::u16(0x1111));
  c.assign("y", literal::u16(2));

  c.mulAssign("x", "y");           // 0x1111 * 2 = 0x2222 -> ""
  c.write("x");                 // ""
  c.assign("z", c.mul("x", "y"));  // 0x2222 * 2 = 0x4444 -> DD
  c.write("z");                 // DD

  c.returnFromFunction();
} c.endFunction();

TEST_END
