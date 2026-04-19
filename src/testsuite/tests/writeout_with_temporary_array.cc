// Tests if we can pass an arraystored in a temporary slot to writeOut
// Expect: "ABCD"

TEST_BEGIN
c.beginFunction("main"); {
  c.beginBlock("entry"); {
    c.writeOut(values::array(TypeSystem::i8(),
			     values::i8('A'),
			     values::i8('B'),
			     values::i8('C'),
			     values::i8('D')));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
