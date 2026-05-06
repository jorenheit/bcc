// Filename: integer_division_i16.cc
// Divide i16 integers using div and divAssign
// Expected: AACC

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::i16());
  c.declareLocal("y", ts::i16());
  c.declareLocal("z", ts::i16());

  c.assign("x", literal::i16(0x8282));
  c.assign("y", literal::i16(2));

  c.divAssign("x", "y");           // 0x8282 / 2 = 0x4141 -> AA
  c.writeOut("x");                 // AA

  c.assign("x", literal::i16(0x8686));
  c.assign("z", c.div("x", "y"));  // 0x8686 / 2 = 0x4343 -> CC
  c.writeOut("z");                 // CC

  c.returnFromFunction();
} c.endFunction();

TEST_END
