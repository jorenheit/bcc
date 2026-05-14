// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Tests passing an array of pointers to a function.
// foo reads through both pointers, then writes new values through them.
// Caller observes the updated locals afterward.
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

  c.callFunction("foo").arg("p").done();
  c.write("a");
  c.write("b");
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("p", u8pa).ret(ts::void_t()).begin(); {
  auto p0Deref = c.dereferencePointer(c.arrayElement("p", 0));
  auto p1Deref = c.dereferencePointer(c.arrayElement("p", 1));

  c.write(p0Deref);
  c.write(p1Deref);

  c.assign(p0Deref, literal::u8('X'));
  c.assign(p1Deref, literal::u8('Y'));

  c.returnFromFunction();
} c.endFunction();

TEST_END
