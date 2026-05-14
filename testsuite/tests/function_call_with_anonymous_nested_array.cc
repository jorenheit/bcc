// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Tests passing a nested anonymous array by value to a function
// Expect: ABCD

TEST_BEGIN

auto array2 = ts::array(ts::u8(), 2);
auto array22 = ts::array(array2, 2);

c.function("main").begin(); {
  auto ab = literal::array(ts::array(ts::u8(), 2)).push(literal::u8('A')).push(literal::u8('B')).done();
  auto cd = literal::array(ts::array(ts::u8(), 2)).push(literal::u8('C')).push(literal::u8('D')).done();

  c.callFunction("foo").arg(literal::array(ts::array(array2, 2)).push(ab).push(cd).done()).done();
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("arr", array22).ret(ts::void_t()).begin(); {
  c.write("arr");
  c.returnFromFunction();
} c.endFunction();

TEST_END
