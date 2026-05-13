// Signed negate expression sanity
// Expected: ABCDEF

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("a", ts::s8());
  c.declareLocal("b", ts::s16());

  // 1. negate(s8 slot): -(-1) = 1; 1 + '@' = 'A'
  c.assign("a", literal::s8(-1));
  c.write(c.add(c.negate("a"), literal::s8('@'))); // A

  // 2. negate(s8 literal): -(2) = -2; -2 + 'D' = 'B'
  c.write(c.add(c.negate(literal::s8(2)), literal::s8('D'))); // B

  // 3. negate(s8 negative literal): -(-3) = 3; 3 + '@' = 'C'
  c.write(c.add(c.negate(literal::s8(-3)), literal::s8('@'))); // C

  // 4. negate(s16 slot): -(300) = -300
  // CAT('p', 'F') - 300 = CAT('D', 'E') -> DE
  c.assign("b", literal::s16(300));
  c.write(c.add(c.negate("b"), literal::s16(CAT('p', 'F')))); // DE

  // 5. negate(s16 literal): -(-1) = 1
  // CAT('E', 'F') + 1 = CAT('F', 'F') -> FF
  c.write(c.add(c.negate(literal::s16(-1)), literal::s16(CAT('E', 'F')))); // FF

  c.returnFromFunction();
} c.endFunction();

TEST_END
