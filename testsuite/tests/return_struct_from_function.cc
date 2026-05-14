// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Return an entire struct object from a function
// Expected: "QR"

TEST_BEGIN
auto point = ts::defineStruct("Point").field("x", ts::u8()).field("y", ts::u8()).done();

c.function("main").begin(); {
  c.declareLocal("s", point);

  c.callFunction("makePoint").into("s").done();
  c.write("s");
  c.returnFromFunction();
} c.endFunction();

c.function("makePoint").ret(point).begin(); {
  c.declareLocal("p", point);
  auto x = c.structField("p", "x");
  auto y = c.structField("p", "y");

  c.assign(x, literal::u8('Q'));
  c.assign(y, literal::u8('R'));
  c.returnFromFunction("p");
} c.endFunction();

TEST_END
