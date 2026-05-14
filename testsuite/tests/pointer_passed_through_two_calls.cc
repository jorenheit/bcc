// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Tests repeated pointer-depth adjustment across two call boundaries.
// main creates a pointer to x, foo forwards it to bar, and bar reads/writes through it.
// Expected: AX

TEST_BEGIN

auto u8  = ts::u8();
auto u8p = ts::pointer(u8);

c.function("main").begin(); {
  c.declareLocal("p", u8p);
  c.declareLocal("x", u8);

  c.assign("x", literal::u8('A'));
  c.assign("p", c.addressOf("x"));
  c.callFunction("foo").arg("p").done();
  c.write("x");
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("p", u8p).ret(ts::void_t()).begin(); {
  c.callFunction("bar").arg("p").done();
  c.returnFromFunction();
} c.endFunction();

c.function("bar").param("p", u8p).ret(ts::void_t()).begin(); {
  auto pDeref = c.dereferencePointer("p");
  c.write(pDeref);
  c.assign(pDeref, literal::u8('X'));
  c.returnFromFunction();
} c.endFunction();

TEST_END
