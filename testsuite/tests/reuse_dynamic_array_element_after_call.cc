// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Write to a runtime-selected array element, make a function call, then read the same element again.
// Expected: "X"

TEST_BEGIN

auto array4 = ts::array(ts::u8(), 4);

c.function("main").begin(); {
  c.declareLocal("arr", array4);
  c.declareLocal("idx", ts::u8());
  c.declareLocal("out", ts::u8());

  c.assign("arr", literal::array(ts::array(ts::u8(), 4)).push(literal::u8('A')).push(literal::u8('B')).push(literal::u8('C')).push(literal::u8('D')).done());
  c.assign("idx", literal::u8(1));

  auto elem = c.arrayElement("arr", "idx");
  c.assign(elem, literal::u8('X'));

  c.callFunction("noop").done();
  auto elemAfterCall = c.arrayElement("arr", "idx");
  c.assign("out", elemAfterCall);
  c.write("out");
  c.returnFromFunction();
} c.endFunction();

c.function("noop").begin(); {
  c.returnFromFunction();
} c.endFunction();

TEST_END
