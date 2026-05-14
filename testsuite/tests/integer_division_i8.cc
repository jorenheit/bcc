// Filename: integer_division_u8.cc
// Divide u8 integers using div and divAssign
// Expected: !#

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::u8());
  c.declareLocal("y", ts::u8());
  c.declareLocal("z", ts::u8());

  c.assign("x", literal::u8(99));
  c.assign("y", literal::u8(3));

  c.divAssign("x", "y");           // 99 / 3 = 33 -> !
  c.write("x");                 // !

  c.assign("x", literal::u8(105));
  c.assign("z", c.div("x", "y"));  // 105 / 3 = 35 -> #
  c.write("z");                 // #

  c.returnFromFunction();
} c.endFunction();

TEST_END
