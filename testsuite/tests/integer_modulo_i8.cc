// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Filename: integer_modulo_u8.cc
// Mod u8 integers using mod and modAssign
// Expected:  !

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::u8());
  c.declareLocal("y", ts::u8());
  c.declareLocal("z", ts::u8());

  c.assign("x", literal::u8(100));
  c.assign("y", literal::u8(34));

  c.modAssign("x", "y");           // 100 % 34 = 32 -> space
  c.write("x");                 // ' '

  c.assign("x", literal::u8(101));
  c.assign("z", c.mod("x", "y"));  // 101 % 34 = 33 -> !
  c.write("z");                 // !

  c.returnFromFunction();
} c.endFunction();

TEST_END
