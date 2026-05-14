// Filename: integer_division_mixed_u16_u8.cc
// Divide mixed u16/u8 integers using div and divAssign
// Expected: AACC

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::u16());
  c.declareLocal("y", ts::u8());
  c.declareLocal("z", ts::u16());

  c.assign("x", literal::u16(0x8282));
  c.assign("y", literal::u8(2));

  c.divAssign("x", "y");           // 0x8282 / 2 = 0x4141 -> AA
  c.write("x");                 // AA

  c.assign("x", literal::u16(0x8686));
  c.assign("z", c.div("x", "y"));  // 0x8686 / 2 = 0x4343 -> CC
  c.write("z");                 // CC

  c.returnFromFunction();
} c.endFunction();

TEST_END
