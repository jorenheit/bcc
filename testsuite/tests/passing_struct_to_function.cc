// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Passing entire struct object to function
// Expected: ABABXYAB

TEST_BEGIN

auto point = ts::defineStruct("Point").field("x", ts::u8()).field("y", ts::u8()).done();

c.function("main").begin(); {
  c.declareLocal("s", point);

  auto x = c.structField("s", "x");
  auto y = c.structField("s", "y");

  c.assign(x, literal::u8('A'));
  c.assign(y, literal::u8('B'));
  c.write("s");

  c.callFunction("foo").arg("s").done();
  c.write("s");
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("p", point).ret(ts::void_t()).begin(); {
  auto px = c.structField("p", "x");
  auto py = c.structField("p", "y");

  c.write("p");
  c.assign(px, literal::u8('X'));
  c.assign(py, literal::u8('Y'));
  c.write("p");
  c.returnFromFunction();
} c.endFunction();

TEST_END
