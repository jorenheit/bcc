// Subtract i16 constants using sub and subAssign
// Expected: AGADAA

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::i16());
  c.declareLocal("z", ts::i16());

  c.assign("x", literal::i16(CAT('G', 'A')));

  c.write("x");                           // GA
  c.subAssign("x", literal::i16(3));          // x -= 3
  c.write("x");                           // DA
  c.assign("z", c.sub("x", literal::i16(3))); // z = x - 3
  c.write("z");                           // AA

  c.returnFromFunction();
} c.endFunction();

TEST_END
