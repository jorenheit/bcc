// Write through two levels of dynamic arrayElement indirection: matrix[row][col].
// Expected: "ABXD"

TEST_BEGIN

auto array2 = ts::array(ts::i8(), 2);
auto matrix2x2 = ts::array(array2, 2);

c.beginFunction("main"); {
  c.declareLocal("m", matrix2x2);
  c.declareLocal("row", ts::i8());
  c.declareLocal("col", ts::i8());

  c.beginBlock("entry"); {
    auto ab = literal::array(ts::i8())(literal::i8('A'),
					      literal::i8('B'));
    auto cd = literal::array(ts::i8())(literal::i8('C'),
					      literal::i8('D'));
    c.assign("m", literal::array(array2)(ab, cd));

    c.assign("row", literal::i8(1));
    c.assign("col", literal::i8(0));

    auto rowRef = c.arrayElement("m", "row");
    auto cellRef = c.arrayElement(rowRef, "col");
    c.assign(cellRef, literal::i8('X'));

    c.writeOut("m");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
