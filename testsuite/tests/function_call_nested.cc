// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Tests forwarding an u8 parameter through a nested call chain (main -> foo -> bar).
// Expect: A

TEST_BEGIN
c.function("main").begin(); {
  c.callFunction("foo").arg(literal::u8('A')).done();
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("x", ts::u8()).ret(ts::void_t()).begin(); {
  c.callFunction("bar").arg("x").done();
  c.returnFromFunction();
} c.endFunction();

c.function("bar").param("y", ts::u8()).ret(ts::void_t()).begin(); {
  c.write("y");
  c.returnFromFunction();
} c.endFunction();

TEST_END
