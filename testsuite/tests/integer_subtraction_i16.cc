// Subtract u16 integers using sub and subAssign
// Expected: AGADAA

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::u16());
  c.declareLocal("y", ts::u16());
  c.declareLocal("z", ts::u16());

  c.assign("x", literal::u16(CAT('G', 'A')));
  c.assign("y", literal::u16(3));

  c.write("x");                    // AG
  c.subAssign("x", "y");              // x -= y
  c.write("x");                    // AD
  c.assign("z", c.sub("x", "y"));     // z = x - y
  c.write("z");                    // AA

  c.returnFromFunction();
} c.endFunction();

TEST_END
