// Subtract i8 integers using sub and subAssign
// Expected: GDA

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::i8());
  c.declareLocal("y", ts::i8());
  c.declareLocal("z", ts::i8());

  c.block("entry").begin(); {
    c.assign("x", literal::i8('G'));
    c.assign("y", literal::i8(3));

    c.writeOut("x");                    // G
    c.subAssign("x", "y");              // x -= y
    c.writeOut("x");                    // D
    c.assign("z", c.sub("x", "y"));     // z = x - y
    c.writeOut("z");                    // A

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
