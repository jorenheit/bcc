// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Signed s8 unary sanity: negate, abs, signBit
// Expected: ABCDEFGHIJKL

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::s8());
  c.declareLocal("z", ts::s8());
  c.declareLocal("f", ts::s8());

  // negate(-1) = 1; 1 + '@' = 'A'
  c.assign("x", literal::s8(-1));
  c.assign("z", c.negate("x"));
  c.write(c.add("z", literal::s8('@'))); // A

  // negate(2) = -2; -2 + 'D' = 'B'
  c.assign("x", literal::s8(2));
  c.assign("z", c.negate("x"));
  c.write(c.add("z", literal::s8('D'))); // B

  // negateAssign(-3) = 3; 3 + '@' = 'C'
  c.assign("x", literal::s8(-3));
  c.negateAssign("x");
  c.write(c.add("x", literal::s8('@'))); // C

  // negateAssign(4) = -4; -4 + 'H' = 'D'
  c.assign("x", literal::s8(4));
  c.negateAssign("x");
  c.write(c.add("x", literal::s8('H'))); // D

  // abs(-5) = 5; 5 + '@' = 'E'
  c.assign("x", literal::s8(-5));
  c.assign("z", c.abs("x"));
  c.write(c.add("z", literal::s8('@'))); // E

  // abs(6) = 6; 6 + '@' = 'F'
  c.assign("x", literal::s8(6));
  c.assign("z", c.abs("x"));
  c.write(c.add("z", literal::s8('@'))); // F

  // absAssign(-7) = 7; 7 + '@' = 'G'
  c.assign("x", literal::s8(-7));
  c.absAssign("x");
  c.write(c.add("x", literal::s8('@'))); // G

  // absAssign(8) = 8; 8 + '@' = 'H'
  c.assign("x", literal::s8(8));
  c.absAssign("x");
  c.write(c.add("x", literal::s8('@'))); // H

  // signBit(-1) = 1; 1 + 'H' = 'I'
  c.assign("x", literal::s8(-1));
  c.assign("f", c.cast(c.signBit("x"), ts::s8()));
  c.write(c.add("f", literal::s8('H'))); // I

  // signBit(0) = 0; 0 + 'J' = 'J'
  c.assign("x", literal::s8(0));
  c.assign("f", c.cast(c.signBit("x"), ts::s8()));
  c.write(c.add("f", literal::s8('J'))); // J

  // signBitAssign(-128): x becomes 1; 1 + 'J' = 'K'
  c.assign("x", literal::s8(-128));
  c.signBitAssign("x");
  c.write(c.add("x", literal::s8('J'))); // K

  // signBitAssign(127): x becomes 0; 0 + 'L' = 'L'
  c.assign("x", literal::s8(127));
  c.signBitAssign("x");
  c.write(c.add("x", literal::s8('L'))); // L

  c.returnFromFunction();
} c.endFunction();

TEST_END
