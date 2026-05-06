// Assign a runtime value into an array element selected by a runtime index.
// Expected: "AYCD"

TEST_BEGIN

auto array4 = ts::array(ts::i8(), 4);

c.function("main").begin(); {
  c.declareLocal("arr", array4);
  c.declareLocal("idx", ts::i8());
  c.declareLocal("value", ts::i8());

  c.assign("arr", literal::array(ts::array(ts::i8(), 4)).push(literal::i8('A')).push(literal::i8('B')).push(literal::i8('C')).push(literal::i8('D')).done());
  c.assign("idx", literal::i8(1));
  c.assign("value", literal::i8('Y'));

  auto elem = c.arrayElement("arr", "idx");
  c.assign(elem, "value");

  c.writeOut("arr");
  c.returnFromFunction();
} c.endFunction();

TEST_END
