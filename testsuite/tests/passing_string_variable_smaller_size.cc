// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Pass "Hello World" anonymously to a function that expects a bigger one
// Expected: "Hello World"


TEST_BEGIN
std::string str = "Hello World";

c.function("main").begin(); {
  c.callFunction("print").arg(literal::string(str)).done();
  c.returnFromFunction();
} c.endFunction();

c.function("print").param("s", ts::string(str.size() * 2)).ret(ts::void_t()).begin(); {
  c.print("s");
  c.returnFromFunction();
} c.endFunction();

TEST_END
