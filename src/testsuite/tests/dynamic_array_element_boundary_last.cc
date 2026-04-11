// Write to the last array element using a runtime index at the upper boundary.
// Expected: "ABCZ"

TEST_BEGIN

auto array4 = TypeSystem::array(TypeSystem::i8(), 4);

c.beginFunction("main"); {
  c.declareLocal("arr", array4);
  c.declareLocal("idx", TypeSystem::i8());

  c.beginBlock("entry"); {
    c.assign("arr", values::array(TypeSystem::i8(), 'A', 'B', 'C', 'D'));
    c.assign("idx", values::i8(3));

    auto elem = c.arrayElement("arr", "idx");
    c.assign(elem, values::i8('Z'));

    c.writeOut("arr");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
