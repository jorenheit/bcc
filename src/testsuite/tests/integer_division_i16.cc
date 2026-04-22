// Filename: integer_division_i16.cc
// Divide i16 integers using div and divAssign
// Expected: AACC

TEST_BEGIN

c.beginFunction("main"); {
  c.declareLocal("x", TypeSystem::i16());
  c.declareLocal("y", TypeSystem::i16());
  c.declareLocal("z", TypeSystem::i16());

  c.beginBlock("entry"); {
    c.assign("x", values::i16(0x8282));
    c.assign("y", values::i16(2));

    c.divAssign("x", "y");           // 0x8282 / 2 = 0x4141 -> AA
    c.writeOut("x");                 // AA

    c.assign("x", values::i16(0x8686));
    c.assign("z", c.div("x", "y"));  // 0x8686 / 2 = 0x4343 -> CC
    c.writeOut("z");                 // CC

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
