// Return a value from a function into a struct field reached through a dynamic array element.
// Expected: "MNOQ"

TEST_BEGIN

auto point = c.defineStruct("Point",
                            "x", TypeSystem::i8(),
                            "y", TypeSystem::i8());
auto pointArray2 = TypeSystem::array(point, 2);

c.beginFunction("main"); {
  c.declareLocal("pts", pointArray2);
  c.declareLocal("idx", TypeSystem::i8());

  c.beginBlock("entry"); {
    auto p0 = values::structT(point, values::i8('M'), values::i8('N'));
    auto p1 = values::structT(point, values::i8('O'), values::i8('P'));
    c.assign("pts", values::array(point, p0, p1));

    c.assign("idx", values::i8(1));

    auto selectedPoint = c.arrayElement("pts", "idx");
    auto dest = c.structField(selectedPoint, "y");
    c.callFunctionReturn("makeQ", "after_makeQ", dest);
  } c.endBlock();

  c.beginBlock("after_makeQ"); {
    c.writeOut("pts");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("makeQ", TypeSystem::i8()); {
  c.beginBlock("entry"); {
    c.returnFromFunction(values::i8('Q'));
  } c.endBlock();
} c.endFunction();

TEST_END
