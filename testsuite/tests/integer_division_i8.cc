// Filename: integer_division_i8.cc
// Divide i8 integers using div and divAssign
// Expected: !#

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::i8());
  c.declareLocal("y", ts::i8());
  c.declareLocal("z", ts::i8());

  c.assign("x", literal::i8(99));
  c.assign("y", literal::i8(3));

  c.divAssign("x", "y");           // 99 / 3 = 33 -> !
  c.write("x");                 // !

  c.assign("x", literal::i8(105));
  c.assign("z", c.div("x", "y"));  // 105 / 3 = 35 -> #
  c.write("z");                 // #

  c.returnFromFunction();
} c.endFunction();

TEST_END
