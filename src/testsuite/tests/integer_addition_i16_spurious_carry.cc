// Add 1 to an i16 value without carry into the high byte
// Expected: ACBC

TEST_BEGIN

c.beginFunction("main"); {
  c.declareLocal("x", TypeSystem::i16());

  c.beginBlock("entry"); {
    c.assign("x", values::i16(CAT('A', 'C')));

    c.writeOut("x");                  // AC
    c.addAssign("x", values::i16(1)); // should become BC
    c.writeOut("x");                  // BC

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
