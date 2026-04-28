// Use a static outer array index and a runtime inner array index in a nested array.
// Expected: "ABQD"

TEST_BEGIN

auto inner = ts::array(ts::i8(), 2);
auto outer = ts::array(inner, 2);

c.beginFunction("main"); {
  c.declareLocal("m", outer);
  c.declareLocal("idx", ts::i8());

  c.beginBlock("entry"); {
    auto ab = literal::array(ts::i8())(literal::i8('A'),
					      literal::i8('B'));
    auto cd = literal::array(ts::i8())(literal::i8('C'),
					      literal::i8('D'));
    c.assign("m", literal::array(inner)(ab, cd));

    c.assign("idx", literal::i8(0));

    auto row1 = c.arrayElement("m", 1);
    auto cell = c.arrayElement(row1, "idx");
    c.assign(cell, literal::i8('Q'));

    c.writeOut("m");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
