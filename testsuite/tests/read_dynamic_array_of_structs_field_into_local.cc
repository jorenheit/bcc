// Read a struct field through a runtime-selected array element into a local.
// Expected: "C"

TEST_BEGIN

auto point = c.defineStruct("Point")("x", TypeSystem::i8(),
				     "y", TypeSystem::i8());
auto pointArray3 = TypeSystem::array(point, 3);

c.beginFunction("main"); {
  c.declareLocal("pts", pointArray3);
  c.declareLocal("idx", TypeSystem::i8());
  c.declareLocal("out", TypeSystem::i8());

  c.beginBlock("entry"); {
    auto p0 = values::structT(point, values::i8('A'), values::i8('B'));
    auto p1 = values::structT(point, values::i8('C'), values::i8('D'));
    auto p2 = values::structT(point, values::i8('E'), values::i8('F'));
    c.assign("pts", values::array(point, p0, p1, p2));
    c.assign("idx", values::i8(1));

    auto selected = c.arrayElement("pts", "idx");
    auto x = c.structField(selected, "x");
    c.assign("out", x);

    c.writeOut("out");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
