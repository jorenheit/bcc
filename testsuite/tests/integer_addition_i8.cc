// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Add u8 integers together using add and addAssign
// Expected: ADG

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::u8());
  c.declareLocal("y", ts::u8());
  c.declareLocal("z", ts::u8());

  c.assign("x", literal::u8('A'));
  c.assign("y", literal::u8(3));

  c.write("x"); // A
  c.addAssign("x", "y"); // x += y
  c.write("x"); // D
  c.assign("z", c.add("x", "y")); // z = x + y
  c.write("z"); // G

  c.returnFromFunction();
} c.endFunction();

TEST_END
