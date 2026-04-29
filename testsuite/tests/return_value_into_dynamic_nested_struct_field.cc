// Return a value from a function into a struct field reached through a dynamic array element.
// Expected: "MNOQ"

TEST_BEGIN

auto point = ts::defineStruct("Point").field("x", ts::i8()).field("y", ts::i8()).done();
auto pointArray2 = ts::array(point, 2);

c.function("main").begin(); {
  c.declareLocal("pts", pointArray2);
  c.declareLocal("idx", ts::i8());

  c.block("entry").begin(); {
    auto p0 = literal::struct_t(point).init("x", literal::i8('M')).init("y", literal::i8('N')).done();
    auto p1 = literal::struct_t(point).init("x", literal::i8('O')).init("y", literal::i8('P')).done();
    c.assign("pts", literal::array(ts::array(point, 2)).push(p0).push(p1).done());

    c.assign("idx", literal::i8(1));

    auto selectedPoint = c.arrayElement("pts", "idx");
    auto dest = c.structField(selectedPoint, "y");
    c.callFunction("makeQ", "after_makeQ").into(dest).done();
  } c.endBlock();

  c.block("after_makeQ").begin(); {
    c.writeOut("pts");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("makeQ").ret(ts::i8()).begin(); {
  c.block("entry").begin(); {
    c.returnFromFunction(literal::i8('Q'));
  } c.endBlock();
} c.endFunction();

TEST_END
