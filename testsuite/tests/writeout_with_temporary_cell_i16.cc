// Tests if we can pass a i16 stored in a temporary cell to write
// Expect: "AB"

TEST_BEGIN
c.function("main").begin(); {
  c.write(literal::i16(CAT('A', 'B')));
  c.returnFromFunction();
} c.endFunction();

TEST_END
