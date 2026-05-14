// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Initialize a struct from an anonymous value
// Expected: "AB"

TEST_BEGIN
auto point = ts::defineStruct("Point").field("x", ts::u8()).field("y", ts::u8()).done();

c.function("main").begin(); {
  c.declareLocal("s", point);

  c.assign("s", literal::struct_t(point).init("x", literal::u8('A')).init("y", literal::u8('B')).done());
  c.write("s");
  c.returnFromFunction();
} c.endFunction();

TEST_END
