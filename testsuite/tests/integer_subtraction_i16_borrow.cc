// Subtract i16 integers with borrow into the high byte
// Expected: AB?A=A

TEST_BEGIN

c.beginFunction("main"); {
  c.declareLocal("x", ts::i16());
  c.declareLocal("y", ts::i16());
  c.declareLocal("z", ts::i16());

  c.beginBlock("entry"); {
    c.assign("x", literal::i16(CAT('A', 'C'))); // low='A', high='B'
    c.assign("y", literal::i16(254));

    c.writeOut("x");                    // AC
    c.subAssign("x", "y");              // 
    c.writeOut("x");                    // CB
    c.assign("z", c.sub("x", "y"));     // 
    c.writeOut("z");                    // EA

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END

