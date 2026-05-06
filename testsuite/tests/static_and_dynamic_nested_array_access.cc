// Use a static outer array index and a runtime inner array index in a nested array.
// Expected: "ABQD"

TEST_BEGIN

auto inner = ts::array(ts::i8(), 2);
auto outer = ts::array(inner, 2);

c.function("main").begin(); {
  c.declareLocal("m", outer);
  c.declareLocal("idx", ts::i8());

  auto ab = literal::array(ts::array(ts::i8(), 2)).push(literal::i8('A')).push(literal::i8('B')).done();
  auto cd = literal::array(ts::array(ts::i8(), 2)).push(literal::i8('C')).push(literal::i8('D')).done();
  c.assign("m", literal::array(ts::array(inner, 2)).push(ab).push(cd).done());

  c.assign("idx", literal::i8(0));

  auto row1 = c.arrayElement("m", 1);
  auto cell = c.arrayElement(row1, "idx");
  c.assign(cell, literal::i8('Q'));

  c.writeOut("m");
  c.returnFromFunction();
} c.endFunction();

TEST_END
