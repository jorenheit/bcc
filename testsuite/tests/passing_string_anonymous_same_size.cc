// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Pass the  string "Hello World" anonymously.
// Expected: "Hello World"

TEST_BEGIN

std::string str = "Hello World";
auto string = ts::string(str.size());

c.function("main").begin(); {
  c.callFunction("print").arg(literal::string(str)).done();
  c.returnFromFunction();
} c.endFunction();

c.function("print").param("s", string).ret(ts::void_t()).begin(); {
  c.print("s");
  c.returnFromFunction();
} c.endFunction();

TEST_END
