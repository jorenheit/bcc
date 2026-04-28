// Tests if we can pass a value stored in a temporary cell to writeOut
// Expect: "A"

TEST_BEGIN

c.beginFunction("main"); {
  c.beginBlock("entry"); {
    c.writeOut(literal::i8('A'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
