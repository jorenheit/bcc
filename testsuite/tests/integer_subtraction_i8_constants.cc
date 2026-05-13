// Subtract i8 constants using sub and subAssign
// Expected: GDA

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::i8());
  c.declareLocal("z", ts::i8());

  c.assign("x", literal::i8('G'));

  c.write("x");                          // G
  c.subAssign("x", literal::i8(3));          // x -= 3
  c.write("x");                          // D
  c.assign("z", c.sub("x", literal::i8(3))); // z = x - 3
  c.write("z");                          // A

  c.returnFromFunction();
} c.endFunction();

TEST_END
