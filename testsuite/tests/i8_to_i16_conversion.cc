// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Convert u8 to u16
// Expected: "AA"

TEST_BEGIN

std::string str = "Hello World";

c.function("main").begin(); {
  c.declareLocal("x", ts::u8());
  c.declareLocal("y", ts::u16());

  c.assign("x", literal::u8('A'));
  c.write("x");
  c.assign("y", "x");
  c.write("y");
  c.returnFromFunction();
} c.endFunction();

TEST_END
