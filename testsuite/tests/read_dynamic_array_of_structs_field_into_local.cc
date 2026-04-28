// Read a struct field through a runtime-selected array element into a local.
// Expected: "C"

TEST_BEGIN

auto point = ts::defineStruct("Point")("x", ts::i8(),
				     "y", ts::i8());
auto pointArray3 = ts::array(point, 3);

c.beginFunction("main"); {
  c.declareLocal("pts", pointArray3);
  c.declareLocal("idx", ts::i8());
  c.declareLocal("out", ts::i8());

  c.beginBlock("entry"); {
    auto p0 = literal::structT(point)(literal::i8('A'), literal::i8('B'));
    auto p1 = literal::structT(point)(literal::i8('C'), literal::i8('D'));
    auto p2 = literal::structT(point)(literal::i8('E'), literal::i8('F'));
    c.assign("pts", literal::array(point)(p0, p1, p2));
    c.assign("idx", literal::i8(1));

    auto selected = c.arrayElement("pts", "idx");
    auto x = c.structField(selected, "x");
    c.assign("out", x);

    c.writeOut("out");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
