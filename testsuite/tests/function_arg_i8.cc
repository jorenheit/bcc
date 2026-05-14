// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Tests passing an u8 constant and an u8 local as function arguments, and returning an u8 value into a caller local.
// Expected: AB

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::u8());
  c.declareLocal("y", ts::u8());

  c.assign("x", literal::u8('A'));
  c.callFunction("foo").into("y").arg("x").arg(literal::u8('B')).done();
  c.write("x");
  c.write("y");
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("arg1", ts::u8()).param("arg2", ts::u8()).ret(ts::u8()).begin(); {
  c.returnFromFunction("arg2");
} c.endFunction();

TEST_END
