// Signed i8 sanity: addition, subtraction, and negation-via-zero-minus-x
// Expected: ABCDEF

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::s8());
  c.declareLocal("z", ts::s8());

  c.block("entry").begin(); {
    // 1. Runtime add with negative LHS:
    // -1 + 'B' = 'A'
    c.assign("x", literal::s8(-1));
    c.assign("z", c.add("x", literal::s8('B')));
    c.writeOut("z"); // A

    // 2. addAssign from negative value:
    // -2 + 4 = 2; 2 + '@' = 'B'
    c.assign("x", literal::s8(-2));
    c.addAssign("x", literal::s8(4));
    c.writeOut(c.add("x", literal::s8('@'))); // B

    // 3. Negation using 0 - x:
    // 0 - 2 = -2; -2 + 'E' = 'C'
    c.assign("x", literal::s8(2));
    c.assign("z", c.sub(literal::s8(0), "x"));
    c.writeOut(c.add("z", literal::s8('E'))); // C

    // 4. subAssign underflow:
    // 0 - 1 = -1; -1 + 'E' = 'D'
    c.assign("x", literal::s8(0));
    c.subAssign("x", literal::s8(1));
    c.writeOut(c.add("x", literal::s8('E'))); // D

    // 5. addAssign with negative RHS:
    // 'G' + (-2) = 'E'
    c.assign("x", literal::s8('G'));
    c.addAssign("x", literal::s8(-2));
    c.writeOut("x"); // E

    // 6. subAssign with negative RHS:
    // 'D' - (-2) = 'F'
    c.assign("x", literal::s8('D'));
    c.subAssign("x", literal::s8(-2));
    c.writeOut("x"); // F

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
