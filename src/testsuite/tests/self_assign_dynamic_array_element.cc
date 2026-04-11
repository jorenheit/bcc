// Self-assignment through a runtime-selected array element should leave the value unchanged.
// Expected: "ABCD"

TEST_BEGIN

auto array4 = TypeSystem::array(TypeSystem::i8(), 4);

c.beginFunction("main"); {
  c.declareLocal("arr", array4);
  c.declareLocal("idx", TypeSystem::i8());

  c.beginBlock("entry"); {
    c.assign("arr", values::array(TypeSystem::i8(), 'A', 'B', 'C', 'D'));
    c.assign("idx", values::i8(2));

    auto elem = c.arrayElement("arr", "idx");
    c.assign(elem, elem);

    c.writeOut("arr");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
