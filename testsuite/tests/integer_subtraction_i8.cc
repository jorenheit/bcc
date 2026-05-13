// Subtract i8 integers using sub and subAssign
// Expected: GDA

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::i8());
  c.declareLocal("y", ts::i8());
  c.declareLocal("z", ts::i8());

  c.assign("x", literal::i8('G'));
  c.assign("y", literal::i8(3));

  c.write("x");                    // G
  c.subAssign("x", "y");              // x -= y
  c.write("x");                    // D
  c.assign("z", c.sub("x", "y"));     // z = x - y
  c.write("z");                    // A

  c.returnFromFunction();
} c.endFunction();

TEST_END
