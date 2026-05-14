// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Tests passing u16 arguments (constant and local) and returning an u16 value from the callee.
// Expect: AB
TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::u16());
  c.declareLocal("y", ts::u16());

  c.assign("x", literal::u16(CAT('C', 'D')));
  c.callFunction("foo").into("y").arg("x").arg(literal::u16(CAT('A', 'B'))).done();
  c.write("y");
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("arg1", ts::u16()).param("arg2", ts::u16()).ret(ts::u16()).begin(); {
  c.returnFromFunction("arg2");
} c.endFunction();

TEST_END
