// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Write through two levels of dynamic arrayElement indirection: matrix[row][col].
// Expected: "ABXD"

TEST_BEGIN

auto array2 = ts::array(ts::u8(), 2);
auto matrix2x2 = ts::array(array2, 2);

c.function("main").begin(); {
  c.declareLocal("m", matrix2x2);
  c.declareLocal("row", ts::u8());
  c.declareLocal("col", ts::u8());

  auto ab = literal::array(ts::array(ts::u8(), 2)).push(literal::u8('A')).push(literal::u8('B')).done();
  auto cd = literal::array(ts::array(ts::u8(), 2)).push(literal::u8('C')).push(literal::u8('D')).done();
  c.assign("m", literal::array(ts::array(array2, 2)).push(ab).push(cd).done());

  c.assign("row", literal::u8(1));
  c.assign("col", literal::u8(0));

  auto rowRef = c.arrayElement("m", "row");
  auto cellRef = c.arrayElement(rowRef, "col");
  c.assign(cellRef, literal::u8('X'));

  c.write("m");
  c.returnFromFunction();
} c.endFunction();

TEST_END
