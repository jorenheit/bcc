// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Select an array field from a struct, then write to a runtime-selected element inside that field.
// Expected: "AQCD"

TEST_BEGIN

auto inner = ts::array(ts::u8(), 3);
auto holder = ts::defineStruct("Holder").field("tag", ts::u8()).field("data", inner).done();

c.function("main").begin(); {
  c.declareLocal("h", holder);
  c.declareLocal("idx", ts::u8());

  auto data = literal::array(ts::array(ts::u8(), 3)).push(literal::u8('B')).push(literal::u8('C')).push(literal::u8('D')).done();

  c.assign("h", literal::struct_t(holder).init("tag", literal::u8('A')).init("data", data).done());
  c.assign("idx", literal::u8(0));

  auto dataField = c.structField("h", "data");
  auto elem = c.arrayElement(dataField, "idx");
  c.assign(elem, literal::u8('Q'));

  c.write("h");
  c.returnFromFunction();
} c.endFunction();

TEST_END
