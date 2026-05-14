// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Store the return-value of a function into a struct-field
// Expected: "A"

TEST_BEGIN

auto point = ts::defineStruct("Point").field("x", ts::u8()).field("y", ts::u8()).done();

c.function("main").begin(); {
  c.declareLocal("s", point);

  auto x = c.structField("s", "x");
  auto y = c.structField("s", "y");

  c.assign(x, literal::u8('A'));
  c.callFunction("makeZ").into(y).done();
  c.write("s");
  c.returnFromFunction();
} c.endFunction();

c.function("makeZ").ret(ts::u8()).begin(); {
  c.returnFromFunction(literal::u8('Z'));
} c.endFunction();

TEST_END
