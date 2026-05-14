// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Filename: integer_modulo_mixed_u16_u8.cc
// Mod mixed u16/u8 integers using mod and modAssign
// Expected: AABB

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::u16());
  c.declareLocal("y", ts::u8());
  c.declareLocal("z", ts::u16());

  c.assign("x", literal::u16(100));
  c.assign("y", literal::u8(34));

  c.modAssign("x", "y"); // 100 % 34 = 0x0020
  c.write(c.add("x", literal::u16(CAT('!', 'A')))); // 0x0020 + 0x4121 = 0x4141 -> AA

  c.assign("x", literal::u16(101));
  c.assign("z", c.add(c.mod("x", "y"), literal::u16(CAT('!', 'B')))); // 0x0021 + 0x4221 = 0x4242 -> BB
  c.write("z"); // BB

  c.returnFromFunction();
} c.endFunction();

TEST_END
