// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Filename: integer_literals_u16.cc
// Literal RHS tests for u16 operators and assign-operators
// Expected: AABBCCDDEEAABBCCDDEE

TEST_BEGIN

auto u16 = ts::u16();

c.function("main").begin(); {
  c.declareLocal("x", u16);

  // add with literal -> "AA"
  c.assign("x", literal::u16(0x4140));
  c.write(c.add("x", literal::u16(1)));

  // sub with literal -> "BB"
  c.assign("x", literal::u16(0x4243));
  c.write(c.sub("x", literal::u16(1)));

  // mul with literal, mapped to "CC"
  c.assign("x", literal::u16(0x1111));
  c.write(c.add(c.mul("x", literal::u16(3)), literal::u16(0x1010)));

  // div with literal, mapped to "DD"
  c.assign("x", literal::u16(0x6363));
  c.write(c.add(c.div("x", literal::u16(3)), literal::u16(0x2323)));

  // mod with literal, mapped to "EE"
  c.assign("x", literal::u16(0x7171));
  c.write(c.add(c.mod("x", literal::u16(0x5050)), literal::u16(0x2424)));

  // addAssign with literal -> "AA"
  c.assign("x", literal::u16(0x4140));
  c.addAssign("x", literal::u16(1));
  c.write("x");

  // subAssign with literal -> "BB"
  c.assign("x", literal::u16(0x4243));
  c.subAssign("x", literal::u16(1));
  c.write("x");

  // mulAssign with literal, mapped to "CC"
  c.assign("x", literal::u16(0x1111));
  c.mulAssign("x", literal::u16(3));
  c.write(c.add("x", literal::u16(0x1010)));

  // divAssign with literal, mapped to "DD"
  c.assign("x", literal::u16(0x6363));
  c.divAssign("x", literal::u16(3));
  c.write(c.add("x", literal::u16(0x2323)));

  // modAssign with literal, mapped to "EE"
  c.assign("x", literal::u16(0x7171));
  c.modAssign("x", literal::u16(0x5050));
  c.write(c.add("x", literal::u16(0x2424)));

  c.returnFromFunction();
} c.endFunction();

TEST_END
