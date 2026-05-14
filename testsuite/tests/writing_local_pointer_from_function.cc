// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Tests writing through a pointer that points to a caller local and is passed to a function.
// foo writes 'Y' through p, so x changes from 'X' to 'Y'.
// Expected: YY

TEST_BEGIN

auto u8  = ts::u8();
auto u8p = ts::pointer(u8);

c.function("main").begin(); {
  c.declareLocal("p", u8p);
  c.declareLocal("x", u8);

  c.assign("x", literal::u8('X'));
  c.assign("p", c.addressOf("x"));
  c.callFunction("foo").arg("p").done();
  c.write("x");
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("p", u8p).ret(ts::void_t()).begin(); {
  auto pDeref = c.dereferencePointer("p");
  c.assign(pDeref, literal::u8('Y'));
  c.write(pDeref);
  c.returnFromFunction();
} c.endFunction();

TEST_END
