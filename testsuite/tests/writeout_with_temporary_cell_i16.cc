// Tests if we can pass a u16 stored in a temporary cell to write
// Expect: "AB"

TEST_BEGIN
c.function("main").begin(); {
  c.write(literal::u16(CAT('A', 'B')));
  c.returnFromFunction();
} c.endFunction();

TEST_END
