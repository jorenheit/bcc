// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Two pointers alias the same target and observe the same write
// Expected: XX

TEST_BEGIN

auto u8  = ts::u8();
auto u8p = ts::pointer(u8);

c.function("main").begin(); {
  c.declareLocal("x", u8);
  c.declareLocal("p", u8p);
  c.declareLocal("q", u8p);

  c.assign("x", literal::u8('A'));
  c.assign("p", c.addressOf("x"));
  c.assign("q", c.addressOf("x"));

  c.assign(c.dereferencePointer("p"), literal::u8('X'));

  c.write(c.dereferencePointer("p")); // X
  c.write(c.dereferencePointer("q")); // X

  c.returnFromFunction();
} c.endFunction();

TEST_END
