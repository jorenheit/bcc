// Tests if we can pass an arraystored in a temporary slot to writeOut
// Expect: "ABCD"

TEST_BEGIN
c.beginFunction("main"); {
  c.beginBlock("entry"); {
    c.writeOut(values::array(TypeSystem::i8(), 'A', 'B', 'C', 'D'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
