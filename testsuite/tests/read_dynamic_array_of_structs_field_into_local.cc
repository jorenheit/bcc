// Read a struct field through a runtime-selected array element into a local.
// Expected: "C"

TEST_BEGIN

auto point = ts::defineStruct("Point").field("x", ts::i8()).field("y", ts::i8()).done();
auto pointArray3 = ts::array(point, 3);

c.function("main").begin(); {
  c.declareLocal("pts", pointArray3);
  c.declareLocal("idx", ts::i8());
  c.declareLocal("out", ts::i8());

  c.block("entry").begin(); {
    auto p0 = literal::struct_t(point).init("x", literal::i8('A')).init("y", literal::i8('B')).done();
    auto p1 = literal::struct_t(point).init("x", literal::i8('C')).init("y", literal::i8('D')).done();
    auto p2 = literal::struct_t(point).init("x", literal::i8('E')).init("y", literal::i8('F')).done();
    c.assign("pts", literal::array(ts::array(point, 3)).push(p0).push(p1).push(p2).done());
    c.assign("idx", literal::i8(1));

    auto selected = c.arrayElement("pts", "idx");
    auto x = c.structField(selected, "x");
    c.assign("out", x);

    c.writeOut("out");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
