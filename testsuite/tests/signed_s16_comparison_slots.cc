// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Signed s16 slot/slot comparisons
// Output convention: 'A' = false, 'B' = true.
// Expected: BABAABABBBAB

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::s16());
  c.declareLocal("y", ts::s16());

  // -1 < 0 -> true
  c.assign("x", literal::s16(-1));
  c.assign("y", literal::s16(0));
  c.write(c.add(c.lt("x", "y"), literal::u8('A'))); // B

  // 0 < -1 -> false
  c.write(c.add(c.lt("y", "x"), literal::u8('A'))); // A

  // -300 < -20 -> true
  c.assign("x", literal::s16(-300));
  c.assign("y", literal::s16(-20));
  c.write(c.add(c.lt("x", "y"), literal::u8('A'))); // B

  // -20 < -300 -> false
  c.write(c.add(c.lt("y", "x"), literal::u8('A'))); // A

  // 32767 < -32768 -> false
  c.assign("x", literal::s16(32767));
  c.assign("y", literal::s16(-32768));
  c.write(c.add(c.lt("x", "y"), literal::u8('A'))); // A

  // -32768 < 32767 -> true
  c.write(c.add(c.lt("y", "x"), literal::u8('A'))); // B

  // -1 > 0 -> false
  c.assign("x", literal::s16(-1));
  c.assign("y", literal::s16(0));
  c.write(c.add(c.gt("x", "y"), literal::u8('A'))); // A

  // 0 > -1 -> true
  c.write(c.add(c.gt("y", "x"), literal::u8('A'))); // B

  // -300 <= -300 -> true
  c.assign("x", literal::s16(-300));
  c.assign("y", literal::s16(-300));
  c.write(c.add(c.le("x", "y"), literal::u8('A'))); // B

  // -300 >= -300 -> true
  c.write(c.add(c.ge("x", "y"), literal::u8('A'))); // B

  // -300 >= -20 -> false
  c.assign("x", literal::s16(-300));
  c.assign("y", literal::s16(-20));
  c.write(c.add(c.ge("x", "y"), literal::u8('A'))); // A

  // -20 >= -300 -> true
  c.write(c.add(c.ge("y", "x"), literal::u8('A'))); // B

  c.returnFromFunction();
} c.endFunction();

TEST_END
