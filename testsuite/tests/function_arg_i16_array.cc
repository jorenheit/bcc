// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Tests pass-by-value of an array of u16 values, verifying multi-cell argument copying across a function call.
// Expect: ABCDABCD

TEST_BEGIN
auto array2 = ts::array(ts::u16(), 2);

c.function("main").begin(); {
  c.declareLocal("x", array2);

  auto x0 = c.arrayElement("x", 0);
  auto x1 = c.arrayElement("x", 1);

  c.assign(x0, literal::u16(CAT('A', 'B')));
  c.assign(x1, literal::u16(CAT('C', 'D')));

  c.write("x");
  c.callFunction("foo").arg("x").done();
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("x", array2).ret(ts::void_t()).begin(); {
  c.write("x");
  c.returnFromFunction();
} c.endFunction();

TEST_END
