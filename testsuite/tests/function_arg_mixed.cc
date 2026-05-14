// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Tests mixed parameter packing with u8, u16, and u8 array arguments in a single function call.
// Expect: ABCDEF

TEST_BEGIN
auto array3 = ts::array(ts::u8(), 3);

c.function("main").begin(); {
  c.declareLocal("a", ts::u8());
  c.declareLocal("b", ts::u16());
  c.declareLocal("arr", array3);

  auto arr0 = c.arrayElement("arr", 0);
  auto arr1 = c.arrayElement("arr", 1);
  auto arr2 = c.arrayElement("arr", 2);

  c.assign("a",  literal::u8('A'));
  c.assign("b",  literal::u16(CAT('B', 'C')));
  c.assign(arr0, literal::u8('D'));
  c.assign(arr1, literal::u8('E'));
  c.assign(arr2, literal::u8('F'));

  c.callFunction("foo").arg("a").arg("b").arg("arr").done();
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("p0", ts::u8()).param("p1", ts::u16()).param("p2", array3).ret(ts::void_t()).begin(); {
  c.write("p0");
  c.write("p1");
  c.write("p2");
  c.returnFromFunction();
} c.endFunction();

TEST_END
