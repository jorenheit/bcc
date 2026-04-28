// Tests if we can pass an arraystored in a temporary slot to writeOut
// Expect: "ABCD"

TEST_BEGIN
c.beginFunction("main"); {
  c.beginBlock("entry"); {
    c.writeOut(literal::array(ts::i8())(literal::i8('A'),
					       literal::i8('B'),
					       literal::i8('C'),
					       literal::i8('D')));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
