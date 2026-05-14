// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Use a static outer array index and a runtime inner array index in a nested array.
// Expected: "ABQD"

TEST_BEGIN

auto inner = ts::array(ts::u8(), 2);
auto outer = ts::array(inner, 2);

c.function("main").begin(); {
  c.declareLocal("m", outer);
  c.declareLocal("idx", ts::u8());

  auto ab = literal::array(ts::array(ts::u8(), 2)).push(literal::u8('A')).push(literal::u8('B')).done();
  auto cd = literal::array(ts::array(ts::u8(), 2)).push(literal::u8('C')).push(literal::u8('D')).done();
  c.assign("m", literal::array(ts::array(inner, 2)).push(ab).push(cd).done());

  c.assign("idx", literal::u8(0));

  auto row1 = c.arrayElement("m", 1);
  auto cell = c.arrayElement(row1, "idx");
  c.assign(cell, literal::u8('Q'));

  c.write("m");
  c.returnFromFunction();
} c.endFunction();

TEST_END
