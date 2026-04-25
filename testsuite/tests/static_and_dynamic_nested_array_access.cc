// Use a static outer array index and a runtime inner array index in a nested array.
// Expected: "ABQD"

TEST_BEGIN

auto inner = TypeSystem::array(TypeSystem::i8(), 2);
auto outer = TypeSystem::array(inner, 2);

c.beginFunction("main"); {
  c.declareLocal("m", outer);
  c.declareLocal("idx", TypeSystem::i8());

  c.beginBlock("entry"); {
    auto ab = values::array(TypeSystem::i8(),
			    values::i8('A'),
			    values::i8('B'));
    auto cd = values::array(TypeSystem::i8(),
			    values::i8('C'),
			    values::i8('D'));
    c.assign("m", values::array(inner, ab, cd));

    c.assign("idx", values::i8(0));

    auto row1 = c.arrayElement("m", 1);
    auto cell = c.arrayElement(row1, "idx");
    c.assign(cell, values::i8('Q'));

    c.writeOut("m");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
