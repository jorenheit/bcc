// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Self-assignment through a runtime-selected array element should leave the value unchanged.
// Expected: "ABCD"

TEST_BEGIN

auto array4 = ts::array(ts::u8(), 4);

c.function("main").begin(); {
  c.declareLocal("arr", array4);
  c.declareLocal("idx", ts::u8());

  c.assign("arr", literal::array(ts::array(ts::u8(), 4)).push(literal::u8('A')).push(literal::u8('B')).push(literal::u8('C')).push(literal::u8('D')).done());
  c.assign("idx", literal::u8(2));

  auto elem = c.arrayElement("arr", "idx");
  c.assign(elem, elem);

  c.write("arr");
  c.returnFromFunction();
} c.endFunction();

TEST_END
