// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Signed comparison assign variants
// Output convention for s8 cases: 'A' = false, 'B' = true.
// s16 assign cases emit two bytes.
// Expected: BABABBABBABB

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("a", ts::s8());
  c.declareLocal("b", ts::s8());
  c.declareLocal("x", ts::s16());
  c.declareLocal("y", ts::s16());

  // s8 ltAssign: -1 < 0 -> true
  c.assign("a", literal::s8(-1));
  c.assign("b", literal::s8(0));
  c.ltAssign("a", "b");
  c.write(c.add("a", literal::s8('A'))); // B

  // s8 gtAssign: -1 > 0 -> false
  c.assign("a", literal::s8(-1));
  c.assign("b", literal::s8(0));
  c.gtAssign("a", "b");
  c.write(c.add("a", literal::s8('A'))); // A

  // s8 leAssign: -5 <= -5 -> true
  c.assign("a", literal::s8(-5));
  c.assign("b", literal::s8(-5));
  c.leAssign("a", "b");
  c.write(c.add("a", literal::s8('A'))); // B

  // s8 geAssign: -6 >= -5 -> false
  c.assign("a", literal::s8(-6));
  c.assign("b", literal::s8(-5));
  c.geAssign("a", "b");
  c.write(c.add("a", literal::s8('A'))); // A

  // s16 ltAssign: -300 < -20 -> true
  // true as s16 is 1; CAT('A','B') + 1 => "BB"
  c.assign("x", literal::s16(-300));
  c.assign("y", literal::s16(-20));
  c.ltAssign("x", "y");
  c.write(c.add("x", literal::s16(CAT('A', 'B')))); // BB

  // s16 gtAssign: -300 > -20 -> false
  // false as s16 is 0; CAT('A','B') => "AB"
  c.assign("x", literal::s16(-300));
  c.assign("y", literal::s16(-20));
  c.gtAssign("x", "y");
  c.write(c.add("x", literal::s16(CAT('A', 'B')))); // AB

  // mixed-width ltAssign: s8(-1) < s16(1) -> true
  c.assign("a", literal::s8(-1));
  c.assign("x", literal::s16(1));
  c.ltAssign("a", "x");
  c.write(c.add("a", literal::s8('A'))); // B

  // mixed-width geAssign: s8(-2) >= s16(-1) -> false
  c.assign("a", literal::s8(-2));
  c.assign("x", literal::s16(-1));
  c.geAssign("a", "x");
  c.write(c.add("a", literal::s8('A'))); // A

  // mixed-width gtAssign into s16 lhs: s16(-1) > s8(-2) -> true
  c.assign("x", literal::s16(-1));
  c.assign("a", literal::s8(-2));
  c.gtAssign("x", "a");
  c.write(c.add("x", literal::s16(CAT('A', 'B')))); // BB

  c.returnFromFunction();
} c.endFunction();

TEST_END
