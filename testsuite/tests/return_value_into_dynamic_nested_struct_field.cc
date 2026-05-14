// Return a value from a function into a struct field reached through a dynamic array element.
// Expected: "MNOQ"

TEST_BEGIN

auto point = ts::defineStruct("Point").field("x", ts::u8()).field("y", ts::u8()).done();
auto pointArray2 = ts::array(point, 2);

c.function("main").begin(); {
  c.declareLocal("pts", pointArray2);
  c.declareLocal("idx", ts::u8());

  auto p0 = literal::struct_t(point).init("x", literal::u8('M')).init("y", literal::u8('N')).done();
  auto p1 = literal::struct_t(point).init("x", literal::u8('O')).init("y", literal::u8('P')).done();
  c.assign("pts", literal::array(ts::array(point, 2)).push(p0).push(p1).done());

  c.assign("idx", literal::u8(1));

  auto selectedPoint = c.arrayElement("pts", "idx");
  auto dest = c.structField(selectedPoint, "y");
  c.callFunction("makeQ").into(dest).done();
  c.write("pts");
  c.returnFromFunction();
} c.endFunction();

c.function("makeQ").ret(ts::u8()).begin(); {
  c.returnFromFunction(literal::u8('Q'));
} c.endFunction();

TEST_END
