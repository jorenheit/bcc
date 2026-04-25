// Subtract i8 constants using sub and subAssign
// Expected: GDA

TEST_BEGIN

c.beginFunction("main"); {
  c.declareLocal("x", TypeSystem::i8());
  c.declareLocal("z", TypeSystem::i8());

  c.beginBlock("entry"); {
    c.assign("x", values::i8('G'));

    c.writeOut("x");                          // G
    c.subAssign("x", values::i8(3));          // x -= 3
    c.writeOut("x");                          // D
    c.assign("z", c.sub("x", values::i8(3))); // z = x - 3
    c.writeOut("z");                          // A

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
