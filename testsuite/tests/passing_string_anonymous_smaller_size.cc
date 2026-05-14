// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Store the string "Hello World" in a variable, then pass it to the print function that expects a bigger one
// Expected: "Hello World"


TEST_BEGIN

std::string str = "Hello World";
auto smallString = ts::string(str.size());
auto bigString = ts::string(str.size() * 2);

c.function("main").begin(); {
  c.declareLocal("s", smallString);

  c.assign("s", literal::string(str));
  c.callFunction("print").arg("s").done();
  c.returnFromFunction();
} c.endFunction();

c.function("print").param("s", bigString).ret(ts::void_t()).begin(); {
  c.print("s");
  c.returnFromFunction();
} c.endFunction();

TEST_END
