// Write to the first array element using a runtime index of 0.
// Expected: "ZBCD"

TEST_BEGIN

auto array4 = ts::array(ts::i8(), 4);

c.function("main").begin(); {
  c.declareLocal("arr", array4);
  c.declareLocal("idx", ts::i8());

  c.assign("arr", literal::array(ts::array(ts::i8(), 4)).push(literal::i8('A')).push(literal::i8('B')).push(literal::i8('C')).push(literal::i8('D')).done());
  c.assign("idx", literal::i8(0));

  auto elem = c.arrayElement("arr", "idx");
  c.assign(elem, literal::i8('Z'));

  c.write("arr");
  c.returnFromFunction();
} c.endFunction();

TEST_END
