// Add i16 constants with carry into the high byte
// Expected: EACBAC

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::i16());
  c.declareLocal("z", ts::i16());

  c.block("entry").begin(); {
    c.assign("x", literal::i16(CAT('E', 'A'))); // low='C', high='A'

    c.writeOut("x");                           // EA
    c.addAssign("x", literal::i16(254));        // EA + 254 -> CB
    c.writeOut("x");                           // CB
    c.assign("z", c.add("x", literal::i16(254))); // CB + 254 -> AC
    c.writeOut("z");                           // AC

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
