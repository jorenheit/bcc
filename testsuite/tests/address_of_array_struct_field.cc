// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Take the address of a struct field inside an array element and read/write through it
// Expected: baXc

TEST_BEGIN

auto u8  = ts::u8();
auto u8p = ts::pointer(u8);

auto pairT = ts::defineStruct("Pair").field("x", u8).field("y", u8).done();
auto arrT = ts::array(pairT, 3);

c.function("main").begin(); {
  c.declareLocal("p", u8p);
  c.declareLocal("arr", arrT);

  c.assign(c.structField(c.arrayElement("arr", 0), "y"), literal::u8('a'));
  c.assign(c.structField(c.arrayElement("arr", 1), "y"), literal::u8('b'));
  c.assign(c.structField(c.arrayElement("arr", 2), "y"), literal::u8('c'));

  c.assign("p", c.addressOf(c.structField(c.arrayElement("arr", 1), "y")));

  auto pDeref = c.dereferencePointer("p");
  c.write(pDeref);                                    // b

  c.assign(pDeref, literal::u8('X'));

  c.write(c.structField(c.arrayElement("arr", 0), "y")); // a
  c.write(c.structField(c.arrayElement("arr", 1), "y")); // X
  c.write(c.structField(c.arrayElement("arr", 2), "y")); // c

  c.returnFromFunction();
} c.endFunction();

TEST_END
