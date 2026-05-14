// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Tests pass-by-value of an u8 array argument, verifying full array copy and correct element ordering in caller and callee.
// Expected: ABCDABCD

TEST_BEGIN
auto array4 = ts::array(ts::u8(), 4);

c.function("main").begin(); {
  c.declareLocal("x", array4);

  auto x0 = c.arrayElement("x", 0);
  auto x1 = c.arrayElement("x", 1);
  auto x2 = c.arrayElement("x", 2);
  auto x3 = c.arrayElement("x", 3);

  c.assign(x0, literal::u8('A'));
  c.assign(x1, literal::u8('B'));
  c.assign(x2, literal::u8('C'));
  c.assign(x3, literal::u8('D'));

  c.write("x");
  c.callFunction("foo").arg("x").done();
  c.returnFromFunction();
} c.endFunction();


c.function("foo").param("x", array4).ret(ts::void_t()).begin(); {
  c.write("x");
  c.returnFromFunction();
} c.endFunction();

TEST_END
