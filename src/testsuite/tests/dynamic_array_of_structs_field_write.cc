// Select a struct through a dynamic array index, then write its fields via structField.
// Expected: "ABQRCD"

TEST_BEGIN

auto point = c.defineStruct("Point",
                            "x", TypeSystem::i8(),
                            "y", TypeSystem::i8());
auto pointArray3 = TypeSystem::array(point, 3);

c.beginFunction("main"); {
  c.declareLocal("pts", pointArray3);
  c.declareLocal("idx", TypeSystem::i8());

  c.beginBlock("entry"); {
    auto p0 = values::structT(point, values::i8('A'), values::i8('B'));
    auto p1 = values::structT(point, values::i8('C'), values::i8('D'));
    auto p2 = values::structT(point, values::i8('C'), values::i8('D'));

    c.assign("pts", values::array(point, p0, p1, p2));
    c.assign("idx", values::i8(1));

    auto selectedPoint = c.arrayElement("pts", "idx");
    auto px = c.structField(selectedPoint, "x");
    auto py = c.structField(selectedPoint, "y");

    c.assign(px, values::i8('Q'));
    c.assign(py, values::i8('R'));

    c.writeOut("pts");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
