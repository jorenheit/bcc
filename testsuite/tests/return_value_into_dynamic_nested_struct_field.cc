// Return a value from a function into a struct field reached through a dynamic array element.
// Expected: "MNOQ"

TEST_BEGIN

auto point = ts::defineStruct("Point").field("x", ts::i8()).field("y", ts::i8()).done();
auto pointArray2 = ts::array(point, 2);

c.beginFunction("main"); {
  c.declareLocal("pts", pointArray2);
  c.declareLocal("idx", ts::i8());

  c.beginBlock("entry"); {
    auto p0 = literal::struct_t(point).init("x", literal::i8('M')).init("y", literal::i8('N')).done();
    auto p1 = literal::struct_t(point).init("x", literal::i8('O')).init("y", literal::i8('P')).done();
    c.assign("pts", literal::array(ts::array(point, 2)).push(p0).push(p1).done());

    c.assign("idx", literal::i8(1));

    auto selectedPoint = c.arrayElement("pts", "idx");
    auto dest = c.structField(selectedPoint, "y");
    c.callFunction("makeQ", "after_makeQ").into(dest).done();
  } c.endBlock();

  c.beginBlock("after_makeQ"); {
    c.writeOut("pts");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto sig = ts::function().ret(ts::i8()).done();
c.beginFunction("makeQ", sig); {
  c.beginBlock("entry"); {
    c.returnFromFunction(literal::i8('Q'));
  } c.endBlock();
} c.endFunction();

TEST_END
