// Select a struct through a dynamic array index, then write its fields via structField.
// Expected: "ABQRCD"

TEST_BEGIN

auto point = ts::defineStruct("Point").field("x", ts::i8()).field("y", ts::i8()).done();
auto pointArray3 = ts::array(point, 3);

c.function("main").begin(); {
  c.declareLocal("pts", pointArray3);
  c.declareLocal("idx", ts::i8());

  auto p0 = literal::struct_t(point).init("x", literal::i8('A')).init("y", literal::i8('B')).done();
  auto p1 = literal::struct_t(point).init("x", literal::i8('C')).init("y", literal::i8('D')).done();
  auto p2 = literal::struct_t(point).init("x", literal::i8('C')).init("y", literal::i8('D')).done();

  c.assign("pts", literal::array(ts::array(point, 3)).push(p0).push(p1).push(p2).done());
  c.assign("idx", literal::i8(1));

  auto selectedPoint = c.arrayElement("pts", "idx");
  auto px = c.structField(selectedPoint, "x");
  auto py = c.structField(selectedPoint, "y");

  c.assign(px, literal::i8('Q'));
  c.assign(py, literal::i8('R'));

  c.writeOut("pts");
  c.returnFromFunction();
} c.endFunction();

TEST_END
