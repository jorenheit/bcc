// Subtract i8 constants using sub and subAssign
// Expected: GDA

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::i8());
  c.declareLocal("z", ts::i8());

  c.block("entry").begin(); {
    c.assign("x", literal::i8('G'));

    c.writeOut("x");                          // G
    c.subAssign("x", literal::i8(3));          // x -= 3
    c.writeOut("x");                          // D
    c.assign("z", c.sub("x", literal::i8(3))); // z = x - 3
    c.writeOut("z");                          // A

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
