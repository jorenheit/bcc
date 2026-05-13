// Tests if we can pass an arraystored in a temporary slot to write
// Expect: "ABCD"

TEST_BEGIN
c.function("main").begin(); {
  c.write(literal::array(ts::array(ts::i8(), 4)).push(literal::i8('A')).push(literal::i8('B')).push(literal::i8('C')).push(literal::i8('D')).done());
  c.returnFromFunction();
} c.endFunction();

TEST_END
