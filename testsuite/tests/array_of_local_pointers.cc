// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Tests an array of pointers to locals.
// p[0] points to a, p[1] points to b.
// Reading through both should produce AB, then writing through both changes them to XY.
// Expected: ABXY

TEST_BEGIN

auto u8   = ts::u8();
auto u8p  = ts::pointer(u8);
auto u8pa = ts::array(u8p, 2);

c.function("main").begin(); {
  c.declareLocal("p", u8pa);
  c.declareLocal("a", u8);
  c.declareLocal("b", u8);

  c.assign("a", literal::u8('A'));
  c.assign("b", literal::u8('B'));

  c.assign(c.arrayElement("p", 0), c.addressOf("a"));
  c.assign(c.arrayElement("p", 1), c.addressOf("b"));

  auto p0Deref = c.dereferencePointer(c.arrayElement("p", 0));
  auto p1Deref = c.dereferencePointer(c.arrayElement("p", 1));

  c.write(p0Deref);
  c.write(p1Deref);

  c.assign(p0Deref, literal::u8('X'));
  c.assign(p1Deref, literal::u8('Y'));

  c.write("a");
  c.write("b");

  c.returnFromFunction();
} c.endFunction();

TEST_END
