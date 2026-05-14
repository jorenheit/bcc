// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Add u16 constants with carry into the high byte
// Expected: EACBAC

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::u16());
  c.declareLocal("z", ts::u16());

  c.assign("x", literal::u16(CAT('E', 'A'))); // low='C', high='A'

  c.write("x");                           // EA
  c.addAssign("x", literal::u16(254));        // EA + 254 -> CB
  c.write("x");                           // CB
  c.assign("z", c.add("x", literal::u16(254))); // CB + 254 -> AC
  c.write("z");                           // AC

  c.returnFromFunction();
} c.endFunction();

TEST_END
