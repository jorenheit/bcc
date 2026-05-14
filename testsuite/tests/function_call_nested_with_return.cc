// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Tests nested calls with return values, propagating an u8 argument through foo -> bar and back to main.
// Expect: Z

TEST_BEGIN
c.function("main").begin(); {
  c.declareLocal("r", ts::u8());

  c.callFunction("foo").into("r").arg(literal::u8('Z')).done();
  c.write("r");
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("x", ts::u8()).ret(ts::u8()).begin(); {
  c.declareLocal("tmp", ts::u8());

  c.callFunction("bar").into("tmp").arg("x").done();
  c.returnFromFunction("tmp");
} c.endFunction();

c.function("bar").param("y", ts::u8()).ret(ts::u8()).begin(); {
  c.returnFromFunction("y");
} c.endFunction();

TEST_END
