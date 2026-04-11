// Copy from one runtime-selected array element to another runtime-selected array element.
// Expected: "ABAD"

TEST_BEGIN

auto array4 = TypeSystem::array(TypeSystem::i8(), 4);

c.beginFunction("main"); {
  c.declareLocal("arr", array4);
  c.declareLocal("srcIdx", TypeSystem::i8());
  c.declareLocal("dstIdx", TypeSystem::i8());

  c.beginBlock("entry"); {
    c.assign("arr", values::array(TypeSystem::i8(), 'A', 'B', 'C', 'D'));
    c.assign("srcIdx", values::i8(0));
    c.assign("dstIdx", values::i8(2));

    auto src = c.arrayElement("arr", "srcIdx");
    auto dst = c.arrayElement("arr", "dstIdx");
    c.assign(dst, src);

    c.writeOut("arr");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
