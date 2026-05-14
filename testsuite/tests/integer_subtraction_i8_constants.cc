// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Subtract u8 constants using sub and subAssign
// Expected: GDA

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::u8());
  c.declareLocal("z", ts::u8());

  c.assign("x", literal::u8('G'));

  c.write("x");                          // G
  c.subAssign("x", literal::u8(3));          // x -= 3
  c.write("x");                          // D
  c.assign("z", c.sub("x", literal::u8(3))); // z = x - 3
  c.write("z");                          // A

  c.returnFromFunction();
} c.endFunction();

TEST_END
