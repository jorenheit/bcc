// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Read a struct field through a runtime-selected array element into a local.
// Expected: "C"

TEST_BEGIN

auto point = ts::defineStruct("Point").field("x", ts::u8()).field("y", ts::u8()).done();
auto pointArray3 = ts::array(point, 3);

c.function("main").begin(); {
  c.declareLocal("pts", pointArray3);
  c.declareLocal("idx", ts::u8());
  c.declareLocal("out", ts::u8());

  auto p0 = literal::struct_t(point).init("x", literal::u8('A')).init("y", literal::u8('B')).done();
  auto p1 = literal::struct_t(point).init("x", literal::u8('C')).init("y", literal::u8('D')).done();
  auto p2 = literal::struct_t(point).init("x", literal::u8('E')).init("y", literal::u8('F')).done();
  c.assign("pts", literal::array(ts::array(point, 3)).push(p0).push(p1).push(p2).done());
  c.assign("idx", literal::u8(1));

  auto selected = c.arrayElement("pts", "idx");
  auto x = c.structField(selected, "x");
  c.assign("out", x);

  c.write("out");
  c.returnFromFunction();
} c.endFunction();

TEST_END
