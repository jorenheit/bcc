// Signed comparison literal folding
// Output convention: 'A' = false, 'B' = true.
// Expected: BABABABABBAB

TEST_BEGIN

c.function("main").begin(); {
  // s8 literal/literal

  // -1 < 0 -> true
  c.write(c.add(c.lt(literal::s8(-1), literal::s8(0)),
                   literal::i8('A'))); // B

  // 0 < -1 -> false
  c.write(c.add(c.lt(literal::s8(0), literal::s8(-1)),
                   literal::i8('A'))); // A

  // -5 < -2 -> true
  c.write(c.add(c.lt(literal::s8(-5), literal::s8(-2)),
                   literal::i8('A'))); // B

  // -2 < -5 -> false
  c.write(c.add(c.lt(literal::s8(-2), literal::s8(-5)),
                   literal::i8('A'))); // A

  // -128 < 127 -> true
  c.write(c.add(c.lt(literal::s8(-128), literal::s8(127)),
                   literal::i8('A'))); // B

  // 127 < -128 -> false
  c.write(c.add(c.lt(literal::s8(127), literal::s8(-128)),
                   literal::i8('A'))); // A

  // s16 literal/literal

  // -300 <= -300 -> true
  c.write(c.add(c.le(literal::s16(-300), literal::s16(-300)),
                   literal::i8('A'))); // B

  // -300 >= -20 -> false
  c.write(c.add(c.ge(literal::s16(-300), literal::s16(-20)),
                   literal::i8('A'))); // A

  // -20 >= -300 -> true
  c.write(c.add(c.ge(literal::s16(-20), literal::s16(-300)),
                   literal::i8('A'))); // B

  // mixed-width signed literal/literal

  // s8(-1) < s16(1) -> true
  c.write(c.add(c.lt(literal::s8(-1), literal::s16(1)),
                   literal::i8('A'))); // B

  // s16(-1) < s8(-2) -> false
  c.write(c.add(c.lt(literal::s16(-1), literal::s8(-2)),
                   literal::i8('A'))); // A

  // s16(-1) > s8(-2) -> true
  c.write(c.add(c.gt(literal::s16(-1), literal::s8(-2)),
                   literal::i8('A'))); // B

  c.returnFromFunction();
} c.endFunction();

TEST_END
