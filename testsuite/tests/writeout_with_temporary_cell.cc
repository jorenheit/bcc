// Tests if we can pass a value stored in a temporary cell to writeOut
// Expect: "A"

TEST_BEGIN

c.function("main").begin(); {
  c.writeOut(literal::i8('A'));
  c.returnFromFunction();
} c.endFunction();

TEST_END
