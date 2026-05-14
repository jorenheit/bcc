// Subtract u8 integers using sub and subAssign
// Expected: GDA

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::u8());
  c.declareLocal("y", ts::u8());
  c.declareLocal("z", ts::u8());

  c.assign("x", literal::u8('G'));
  c.assign("y", literal::u8(3));

  c.write("x");                    // G
  c.subAssign("x", "y");              // x -= y
  c.write("x");                    // D
  c.assign("z", c.sub("x", "y"));     // z = x - y
  c.write("z");                    // A

  c.returnFromFunction();
} c.endFunction();

TEST_END
