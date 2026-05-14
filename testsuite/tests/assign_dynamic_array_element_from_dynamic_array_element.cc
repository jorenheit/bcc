// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Copy from one runtime-selected array element to another runtime-selected array element.
// Expected: "ABAD"

TEST_BEGIN

auto array4 = ts::array(ts::u8(), 4);

c.function("main").begin(); {
  c.declareLocal("arr", array4);
  c.declareLocal("srcIdx", ts::u8());
  c.declareLocal("dstIdx", ts::u8());

  c.assign("arr", literal::array(ts::array(ts::u8(), 4)).push(literal::u8('A')).push(literal::u8('B')).push(literal::u8('C')).push(literal::u8('D')).done());
  c.assign("srcIdx", literal::u8(0));
  c.assign("dstIdx", literal::u8(2));

  auto src = c.arrayElement("arr", "srcIdx");
  auto dst = c.arrayElement("arr", "dstIdx");
  c.assign(dst, src);

  c.write("arr");
  c.returnFromFunction();
} c.endFunction();

TEST_END
