// Filename: integer_division_i8.cc
// Divide i8 integers using div and divAssign
// Expected: !#

TEST_BEGIN

c.beginFunction("main"); {
  c.declareLocal("x", TypeSystem::i8());
  c.declareLocal("y", TypeSystem::i8());
  c.declareLocal("z", TypeSystem::i8());

  c.beginBlock("entry"); {
    c.assign("x", values::i8(99));
    c.assign("y", values::i8(3));

    c.divAssign("x", "y");           // 99 / 3 = 33 -> !
    c.writeOut("x");                 // !

    c.assign("x", values::i8(105));
    c.assign("z", c.div("x", "y"));  // 105 / 3 = 35 -> #
    c.writeOut("z");                 // #

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
