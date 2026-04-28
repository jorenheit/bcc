// Subtract i16 integers using sub and subAssign
// Expected: AGADAA

TEST_BEGIN

c.beginFunction("main"); {
  c.declareLocal("x", ts::i16());
  c.declareLocal("y", ts::i16());
  c.declareLocal("z", ts::i16());

  c.beginBlock("entry"); {
    c.assign("x", literal::i16(CAT('G', 'A')));
    c.assign("y", literal::i16(3));

    c.writeOut("x");                    // AG
    c.subAssign("x", "y");              // x -= y
    c.writeOut("x");                    // AD
    c.assign("z", c.sub("x", "y"));     // z = x - y
    c.writeOut("z");                    // AA

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
