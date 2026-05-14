// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Do pointer arithmetic on an u8 array using add, addAssign and subAssign
// Expected: ACBB

TEST_BEGIN

auto u8   = ts::u8();
auto u8p  = ts::pointer(u8);
auto arrT = ts::array(u8, 4);

c.function("main").begin(); {
  c.declareLocal("p", u8p);
  c.declareLocal("q", u8p);
  c.declareLocal("arr", arrT);

  c.assign(c.arrayElement("arr", 0), literal::u8('A'));
  c.assign(c.arrayElement("arr", 1), literal::u8('B'));
  c.assign(c.arrayElement("arr", 2), literal::u8('C'));
  c.assign(c.arrayElement("arr", 3), literal::u8('D'));

  c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
  c.write(c.dereferencePointer("p"));        // A

  c.assign("q", c.add("p", literal::u16(2)));
  c.write(c.dereferencePointer("q"));        // C

  c.addAssign("p", literal::u16(1));
  c.write(c.dereferencePointer("p"));        // B

  c.subAssign("q", literal::u16(1));
  c.write(c.dereferencePointer("q"));        // B

  c.returnFromFunction();
} c.endFunction();

TEST_END
