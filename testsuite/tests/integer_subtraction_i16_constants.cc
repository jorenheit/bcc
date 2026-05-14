// Subtract u16 constants using sub and subAssign
// Expected: AGADAA

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::u16());
  c.declareLocal("z", ts::u16());

  c.assign("x", literal::u16(CAT('G', 'A')));

  c.write("x");                           // GA
  c.subAssign("x", literal::u16(3));          // x -= 3
  c.write("x");                           // DA
  c.assign("z", c.sub("x", literal::u16(3))); // z = x - 3
  c.write("z");                           // AA

  c.returnFromFunction();
} c.endFunction();

TEST_END
