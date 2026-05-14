// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Tests reading through a local pointer and then writing back through it in the same frame.
// x is initialized to 'X', y receives *p, then *p is assigned 'Y'.
// Expected: XXYX

TEST_BEGIN

auto u8  = ts::u8();
auto u8p = ts::pointer(u8);

c.function("main").begin(); {
  c.declareLocal("p", u8p);
  c.declareLocal("x", u8);
  c.declareLocal("y", u8);

  c.assign("p", c.addressOf("x")); //literal::pointer(u8, "x"));
  c.assign("x", literal::u8('X'));

  auto pDeref = c.dereferencePointer("p");
  c.assign("y", pDeref);

  c.write("x");
  c.write("y");

  c.assign(pDeref, literal::u8('Y'));

  c.write("x");
  c.write("y");

  c.returnFromFunction();
} c.endFunction();

TEST_END
