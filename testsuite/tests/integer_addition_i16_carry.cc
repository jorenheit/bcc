// Add i16 integers with carry into the high byte
// Expected: EACBAC

TEST_BEGIN

c.beginFunction("main"); {
  c.declareLocal("x", ts::i16());
  c.declareLocal("y", ts::i16());
  c.declareLocal("z", ts::i16());

  c.beginBlock("entry"); {
    c.assign("x", literal::i16(CAT('E', 'A'))); // low='D', high='A'
    c.assign("y", literal::i16(254));

    c.writeOut("x");                    // EA
    c.addAssign("x", "y");              // EA + 254 -> CB
    c.writeOut("x");                    // CB
    c.assign("z", c.add("x", "y"));     // CB + 254 -> AC
    c.writeOut("z");                    // AC

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
