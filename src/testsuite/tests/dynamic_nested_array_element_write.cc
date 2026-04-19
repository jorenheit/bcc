// Write through two levels of dynamic arrayElement indirection: matrix[row][col].
// Expected: "ABXD"

TEST_BEGIN

auto array2 = TypeSystem::array(TypeSystem::i8(), 2);
auto matrix2x2 = TypeSystem::array(array2, 2);

c.beginFunction("main"); {
  c.declareLocal("m", matrix2x2);
  c.declareLocal("row", TypeSystem::i8());
  c.declareLocal("col", TypeSystem::i8());

  c.beginBlock("entry"); {
    auto ab = values::array(TypeSystem::i8(),
			    values::i8('A'),
			    values::i8('B'));
    auto cd = values::array(TypeSystem::i8(),
			    values::i8('C'),
			    values::i8('D'));
    c.assign("m", values::array(array2, ab, cd));

    c.assign("row", values::i8(1));
    c.assign("col", values::i8(0));

    auto rowRef = c.arrayElement("m", "row");
    auto cellRef = c.arrayElement(rowRef, "col");
    c.assign(cellRef, values::i8('X'));

    c.writeOut("m");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
