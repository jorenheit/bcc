// Signed negateAssign sanity
// Expected: ABCDEFGH

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("a", ts::s8());
  c.declareLocal("b", ts::s16());

  // 1. s8: -(-1) = 1; 1 + '@' = 'A'
  c.assign("a", literal::s8(-1));
  c.negateAssign("a");
  c.write(c.add("a", literal::s8('@'))); // A

  // 2. s8: -(2) = -2; -2 + 'D' = 'B'
  c.assign("a", literal::s8(2));
  c.negateAssign("a");
  c.write(c.add("a", literal::s8('D'))); // B

  // 3. s8: -(-3) = 3; 3 + '@' = 'C'
  c.assign("a", literal::s8(-3));
  c.negateAssign("a");
  c.write(c.add("a", literal::s8('@'))); // C

  // 4. s16: -(300) = -300
  // CAT('p', 'F') - 300 = CAT('D', 'E') -> DE
  c.assign("b", literal::s16(300));
  c.negateAssign("b");
  c.write(c.add("b", literal::s16(CAT('p', 'F')))); // DE


  // 5. s16: -(-300) = 300
  // CAT(',', 'A') + 300 = CAT('X', 'B') -> XB
  c.assign("b", literal::s16(-300));
  c.negateAssign("b");
  c.write(c.add("b", literal::s16(CAT(',', 'A')))); // XB

  // 6. s16: -(-1) = 1
  // CAT('G', 'H') + 1 = CAT('H', 'H') -> HH
  c.assign("b", literal::s16(-1));
  c.negateAssign("b");
  c.write(c.add("b", literal::s16(CAT('G', 'H')))); // HH

  c.returnFromFunction();
} c.endFunction();


TEST_END
