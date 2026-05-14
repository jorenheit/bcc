// Signed u16 sanity: addition, subtraction, and negation-via-zero-minus-x
// Expected: ABABAB

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::s16());
  c.declareLocal("z", ts::s16());

  // 1. Runtime add with negative LHS:
  // -1 + 0x4242 = 0x4241 -> writes low/high: AB
  c.assign("x", literal::s16(-1));
  c.assign("z", c.add("x", literal::s16(CAT('B', 'B'))));
  c.write("z"); // AB

  // 2. Negation using 0 - x:
  // 0 - 1 = -1; -1 + 0x4242 = 0x4241 -> AB
  c.assign("x", literal::s16(1));
  c.assign("z", c.sub(literal::s16(0), "x"));
  c.addAssign("z", literal::s16(CAT('B', 'B')));
  c.write("z"); // AB

  // 3. subAssign borrow:
  // 0x4242 - 1 = 0x4241 -> AB
  c.assign("z", literal::s16(CAT('B', 'B')));
  c.subAssign("z", literal::s16(1));
  c.write("z"); // AB

  c.returnFromFunction();
} c.endFunction();

TEST_END
