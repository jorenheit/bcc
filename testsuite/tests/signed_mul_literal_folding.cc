// Signed multiplication literal folding
// Expected: ABCDEF

TEST_BEGIN

c.function("main").begin(); {
  // -3 * 2 = -6; -6 + 'G' = 'A'
  c.write(c.add(c.mul(literal::s8(-3), literal::s8(2)),
  literal::s8('G'))); // A

  // 7 * -1 = -7; -7 + 'I' = 'B'
  c.write(c.add(c.mul(literal::s8(7), literal::s8(-1)),
  literal::s8('I'))); // B

  // -4 * -4 = 16; 16 + '3' = 'C'
  c.write(c.add(c.mul(literal::s8(-4), literal::s8(-4)),
  literal::s8('3'))); // C

  // 9 * -5 = -45; -45 + 'q' = 'D'
  c.write(c.add(c.mul(literal::s8(9), literal::s8(-5)),
  literal::s8('q'))); // D

  // -11 * -3 = 33; 33 + '$' = 'E'
  c.write(c.add(c.mul(literal::s8(-11), literal::s8(-3)),
  literal::s8('$'))); // E

  // 12 * -2 = -24; -24 + '^' = 'F'
  c.write(c.add(c.mul(literal::s8(12), literal::s8(-2)),
  literal::s8('^'))); // F

  c.returnFromFunction();
} c.endFunction();

TEST_END
