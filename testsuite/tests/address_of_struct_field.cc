// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Take the address of a struct field and read/write through it
// Expected: BAX

TEST_BEGIN

auto u8  = ts::u8();
auto u8p = ts::pointer(u8);

auto pairT = ts::defineStruct("Pair").field("x", u8).field("y", u8).done();

c.function("main").begin(); {
  c.declareLocal("p", u8p);
  c.declareLocal("s", pairT);

  c.assign(c.structField("s", "x"), literal::u8('A'));
  c.assign(c.structField("s", "y"), literal::u8('B'));

  c.assign("p", c.addressOf(c.structField("s", "y")));

  auto pDeref = c.dereferencePointer("p");
  c.write(pDeref);                  // B

  c.assign(pDeref, literal::u8('X'));

  c.write(c.structField("s", "x")); // A
  c.write(c.structField("s", "y")); // X

  c.returnFromFunction();
} c.endFunction();

TEST_END
