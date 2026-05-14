// Tests if we can pass an arraystored in a temporary slot to write
// Expect: "ABCD"

TEST_BEGIN
c.function("main").begin(); {
  c.write(literal::array(ts::array(ts::u8(), 4)).push(literal::u8('A')).push(literal::u8('B')).push(literal::u8('C')).push(literal::u8('D')).done());
  c.returnFromFunction();
} c.endFunction();

TEST_END
