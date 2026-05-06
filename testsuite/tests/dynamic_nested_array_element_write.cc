// Write through two levels of dynamic arrayElement indirection: matrix[row][col].
// Expected: "ABXD"

TEST_BEGIN

auto array2 = ts::array(ts::i8(), 2);
auto matrix2x2 = ts::array(array2, 2);

c.function("main").begin(); {
  c.declareLocal("m", matrix2x2);
  c.declareLocal("row", ts::i8());
  c.declareLocal("col", ts::i8());

  auto ab = literal::array(ts::array(ts::i8(), 2)).push(literal::i8('A')).push(literal::i8('B')).done();
  auto cd = literal::array(ts::array(ts::i8(), 2)).push(literal::i8('C')).push(literal::i8('D')).done();
  c.assign("m", literal::array(ts::array(array2, 2)).push(ab).push(cd).done());

  c.assign("row", literal::i8(1));
  c.assign("col", literal::i8(0));

  auto rowRef = c.arrayElement("m", "row");
  auto cellRef = c.arrayElement(rowRef, "col");
  c.assign(cellRef, literal::i8('X'));

  c.writeOut("m");
  c.returnFromFunction();
} c.endFunction();

TEST_END
