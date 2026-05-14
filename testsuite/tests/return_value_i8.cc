// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Tests basic u8 return-value propagation from callee to caller local.
// Expected: X

TEST_BEGIN

// main: void
c.function("main").begin(); {
  c.declareLocal("x", ts::u8());

  c.callFunction("foo").into("x").done();
  c.write("x");
  c.returnFromFunction();
} c.endFunction();

// foo: returns u8
c.function("foo").ret(ts::u8()).begin(); {

  c.returnFromFunction(literal::u8('X'));
} c.endFunction();

TEST_END
