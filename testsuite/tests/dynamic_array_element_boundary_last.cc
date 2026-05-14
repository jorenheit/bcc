// Write to the last array element using a runtime index at the upper boundary.
// Expected: "ABCZ"

TEST_BEGIN

auto array4 = ts::array(ts::u8(), 4);

c.function("main").begin(); {
  c.declareLocal("arr", array4);
  c.declareLocal("idx", ts::u8());

  c.assign("arr", literal::array(ts::array(ts::u8(), 4)).push(literal::u8('A')).push(literal::u8('B')).push(literal::u8('C')).push(literal::u8('D')).done());
  c.assign("idx", literal::u8(3));

  auto elem = c.arrayElement("arr", "idx");
  c.assign(elem, literal::u8('Z'));

  c.write("arr");
  c.returnFromFunction();
} c.endFunction();

TEST_END
