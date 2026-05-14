// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Do pointer arithmetic on an array of pointers and dereference twice
// Expected: ACB

TEST_BEGIN

auto u8    = ts::u8();
auto u8p   = ts::pointer(u8);
auto u8pp  = ts::pointer(u8p);
auto ptrArrT = ts::array(u8p, 3);

c.function("main").begin(); {
  c.declareLocal("x", u8);
  c.declareLocal("y", u8);
  c.declareLocal("z", u8);
  c.declareLocal("pp", u8pp);
  c.declareLocal("qq", u8pp);
  c.declareLocal("arr", ptrArrT);

  c.assign("x", literal::u8('A'));
  c.assign("y", literal::u8('B'));
  c.assign("z", literal::u8('C'));

  c.assign(c.arrayElement("arr", 0), c.addressOf("x"));
  c.assign(c.arrayElement("arr", 1), c.addressOf("y"));
  c.assign(c.arrayElement("arr", 2), c.addressOf("z"));

  c.assign("pp", c.addressOf(c.arrayElement("arr", 0)));

  auto p0 = c.dereferencePointer("pp");
  c.write(c.dereferencePointer(p0)); // A

  c.assign("qq", c.add("pp", literal::u16(2)));
  auto p2 = c.dereferencePointer("qq");
  c.write(c.dereferencePointer(p2)); // C

  c.subAssign("qq", literal::u16(1));
  auto p1 = c.dereferencePointer("qq");
  c.write(c.dereferencePointer(p1)); // B

  c.returnFromFunction();
} c.endFunction();

TEST_END
