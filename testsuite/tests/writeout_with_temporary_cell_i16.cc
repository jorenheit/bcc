// Tests if we can pass a i16 stored in a temporary cell to writeOut
// Expect: "AB"

TEST_BEGIN
c.beginFunction("main"); {
  c.beginBlock("entry"); {
    c.writeOut(literal::i16(CAT('A', 'B')));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
