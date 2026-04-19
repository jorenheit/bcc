// Subtract i8 integers using sub and subAssign
// Expected: GDA

TEST_BEGIN

c.beginFunction("main"); {
  c.declareLocal("x", TypeSystem::i8());
  c.declareLocal("y", TypeSystem::i8());
  c.declareLocal("z", TypeSystem::i8());

  c.beginBlock("entry"); {
    c.assign("x", values::i8('G'));
    c.assign("y", values::i8(3));

    c.writeOut("x");                    // G
    c.subAssign("x", "y");              // x -= y
    c.writeOut("x");                    // D
    c.assign("z", c.sub("x", "y"));     // z = x - y
    c.writeOut("z");                    // A

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
