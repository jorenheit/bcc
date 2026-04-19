// Add i16 constants with carry into the high byte
// Expected: EACBAC

TEST_BEGIN

c.beginFunction("main"); {
  c.declareLocal("x", TypeSystem::i16());
  c.declareLocal("z", TypeSystem::i16());

  c.beginBlock("entry"); {
    c.assign("x", values::i16(CAT('E', 'A'))); // low='C', high='A'

    c.writeOut("x");                           // EA
    c.addAssign("x", values::i16(254));        // EA + 254 -> CB
    c.writeOut("x");                           // CB
    c.assign("z", c.add("x", values::i16(254))); // CB + 254 -> AC
    c.writeOut("z");                           // AC

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
