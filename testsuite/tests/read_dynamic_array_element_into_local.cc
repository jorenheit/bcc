// Read from an array element selected by a runtime index into a normal local.
// Expected: "C"

TEST_BEGIN

auto array4 = ts::array(ts::u8(), 4);

c.function("main").begin(); {
  c.declareLocal("arr", array4);
  c.declareLocal("idx", ts::u8());
  c.declareLocal("out", ts::u8());

  c.assign("arr", literal::array(ts::array(ts::u8(), 4)).push(literal::u8('A')).push(literal::u8('B')).push(literal::u8('C')).push(literal::u8('D')).done());
  c.assign("idx", literal::u8(2));

  auto elem = c.arrayElement("arr", "idx");
  c.assign("out", elem);

  c.write("out");
  c.returnFromFunction();
} c.endFunction();

TEST_END
