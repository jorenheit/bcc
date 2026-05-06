// Tests if we can pass a i16 stored in a temporary cell to writeOut
// Expect: "AB"

TEST_BEGIN
c.function("main").begin(); {
  c.writeOut(literal::i16(CAT('A', 'B')));
  c.returnFromFunction();
} c.endFunction();

TEST_END
