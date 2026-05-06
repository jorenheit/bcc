// Read from an array element selected by a runtime index into a normal local.
// Expected: "C"

TEST_BEGIN

auto array4 = ts::array(ts::i8(), 4);

c.function("main").begin(); {
  c.declareLocal("arr", array4);
  c.declareLocal("idx", ts::i8());
  c.declareLocal("out", ts::i8());

  c.assign("arr", literal::array(ts::array(ts::i8(), 4)).push(literal::i8('A')).push(literal::i8('B')).push(literal::i8('C')).push(literal::i8('D')).done());
  c.assign("idx", literal::i8(2));

  auto elem = c.arrayElement("arr", "idx");
  c.assign("out", elem);

  c.writeOut("out");
  c.returnFromFunction();
} c.endFunction();

TEST_END
